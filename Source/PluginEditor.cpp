#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NeuralFlowAmpAudioProcessorEditor::NeuralFlowAmpAudioProcessorEditor (NeuralFlowAmpAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    auto setupKnob = [this](juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment,
        const juce::String& paramID, const juce::String& displayName, juce::Colour neonColour)
        {
            slider.setName(displayName);
            slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 45, 18);
            slider.setLookAndFeel(&customTheme);

            slider.setColour(juce::Slider::rotarySliderFillColourId, neonColour);
            slider.setColour(juce::Label::textColourId, neonColour); 

            addAndMakeVisible(slider);
            attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, paramID, slider);
        };

    // 1. MAIN AMP KNOBS
    setupKnob(driveKnob, driveAttachment, "DRIVE", "DRIVE", juce::Colour(255, 140, 0));
    setupKnob(lowKnob, lowAttachment, "LOW", "LOW", juce::Colour(0, 213, 255));
    setupKnob(midKnob, midAttachment, "MID", "MID", juce::Colour(0, 213, 255));
    setupKnob(highKnob, highAttachment, "HIGH", "HIGH", juce::Colour(0, 213, 255));
    setupKnob(masterKnob, masterAttachment, "MASTER", "MASTER", juce::Colour(0, 255, 150));

    // 2. PEDALS AND KNOBS
    addAndMakeVisible(compressorBox);
    addAndMakeVisible(overdriveBox);

    auto compColor = juce::Colour(0, 213, 255);
    setupKnob(compSustainKnob, compSustAtt, "COMP_SUST", "SUSTAIN", compColor);
    setupKnob(compAttackKnob, compAttAtt, "COMP_ATT", "ATTACK", compColor);
    setupKnob(compBlendKnob, compBlendAtt, "COMP_MIX", "BLEND", compColor);
    setupKnob(compLevelKnob, compLvlAtt, "COMP_LVL", "LEVEL", compColor);

    auto odColor = juce::Colour(255, 60, 60);
    setupKnob(odGainKnob, odGainAtt, "OD_GAIN", "GAIN", odColor);
    setupKnob(odToneKnob, odToneAtt, "OD_TONE", "TONE", odColor);
    setupKnob(odLevelKnob, odLvlAtt, "OD_LVL", "LEVEL", odColor);

	// 3. LOAD IR BUTTON
    loadIRButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    loadIRButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);

	addAndMakeVisible(loadIRButton);

    loadIRButton.onClick = [this]() {
        fileChooser = std::make_unique<juce::FileChooser>("Select an IR file (.wav)", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav");
        
		auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    
        fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
			auto file = fc.getResult();
            if (file.existsAsFile()) {
				audioProcessor.loadImpulseResponse(file);

				loadIRButton.setButtonText(file.getFileNameWithoutExtension());
            }
        });
    };

    // 60 FPS
    startTimerHz(60);

    setSize(1600, 900);
}

NeuralFlowAmpAudioProcessorEditor::~NeuralFlowAmpAudioProcessorEditor()
{
	driveKnob.setLookAndFeel(nullptr);
    lowKnob.setLookAndFeel(nullptr);
	midKnob.setLookAndFeel(nullptr);
	highKnob.setLookAndFeel(nullptr);
	masterKnob.setLookAndFeel(nullptr);
    stopTimer();
}

//==============================================================================
void NeuralFlowAmpAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour(12, 12, 12)); 

    auto drawPanel = [&](juce::Rectangle<int> bounds, const juce::String& title, juce::Colour bgColour, bool isInner = false) {
        g.setColour(bgColour);
        g.fillRoundedRectangle(bounds.toFloat(), 6.0f);

        g.setColour(isInner ? juce::Colour(35, 35, 35) : juce::Colour(45, 45, 45));
        g.drawRoundedRectangle(bounds.toFloat(), 6.0f, 1.2f);

        if (title.isNotEmpty() && !isInner) {
            g.setColour(juce::Colours::white.withAlpha(0.8f));
            g.setFont(juce::Font(24.0f, juce::Font::bold));
            g.drawText(title, bounds.withTrimmedLeft(15).withTrimmedTop(12).withHeight(20), juce::Justification::topLeft);
        }
    };  

    drawPanel(leadChannelArea, "LEAD CHANNEL", juce::Colour(22, 22, 22));
    drawPanel(signalChainArea, "SIGNAL CHAIN", juce::Colour(22, 22, 22));
    drawPanel(cabSimArea, "CAB SIMULATION", juce::Colour(22, 22, 22));

    drawPanel(waveformArea, "", juce::Colour(4, 4, 4), true);
    drawPanel(knobPanelArea, "", juce::Colour(14, 16, 20), true);

	// Drawing oscilloscope waveform

    auto scopeRect = waveformArea.reduced(10).toFloat();
    
    juce::Path wavePath;
    
	// A lambda function to map the audio sample value to the Y coordinate in the waveform area
    auto mapToY = [&](float value) {
		// Audio is normalized between -1.0 and 1.0, so we map it to the height of the waveform area
		// %45 of the height is used to leave some padding at the top and bottom
        return scopeRect.getCentreY() - (value * scopeRect.getHeight() * 0.45f); 
    };

    wavePath.startNewSubPath (scopeRect.getX(), mapToY(scopeDataToDraw[0]));

	// Calculate the rest 511 points of the waveform path based on the scope data
    for (int i = 1; i < scopeDataToDraw.size(); ++i)
    {
		// X coordinate is evenly distributed across the width of the waveform area
        float x = scopeRect.getX() + (scopeRect.getWidth() * i / (float)(scopeDataToDraw.size() - 1));
        float y = mapToY(scopeDataToDraw[i]);
        
        wavePath.lineTo (x, y);
    }

    auto neonColour = juce::Colour (0, 213, 255); // Cyber Blue

    // Glow Effect
    g.setColour (neonColour.withAlpha (0.3f));
    g.strokePath (wavePath, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

	// Main Waveform
    g.setColour (neonColour.brighter (0.6f));
    g.strokePath (wavePath, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void NeuralFlowAmpAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(35);

    leadChannelArea = area.removeFromTop(area.getHeight() / 3 - 5);
    area.removeFromTop(25);

    signalChainArea = area.removeFromLeft(area.getWidth() / 1.75 - 5);
    area.removeFromLeft(35); 
    cabSimArea = area;

    // 1. LEAD CHANNEL & WAVEFORM AREA
    auto leadContent = leadChannelArea.withTrimmedTop(35).reduced(15, 15);

    knobPanelArea = leadContent.removeFromRight(512);
    leadContent.removeFromRight(15); 
    waveformArea = leadContent;

    juce::FlexBox flexBox;
    flexBox.flexDirection = juce::FlexBox::Direction::row;
    flexBox.justifyContent = juce::FlexBox::JustifyContent::center;
	flexBox.alignItems = juce::FlexBox::AlignItems::center;

    int kWidth = 75;
    int kHeight = 110;

    juce::FlexItem::Margin knobMargin(0, 12, 0, 12);

    flexBox.items.add(juce::FlexItem(driveKnob).withWidth(kWidth).withHeight(kHeight).withMargin(knobMargin));
    flexBox.items.add(juce::FlexItem(lowKnob).withWidth(kWidth).withHeight(kHeight).withMargin(knobMargin));
    flexBox.items.add(juce::FlexItem(midKnob).withWidth(kWidth).withHeight(kHeight).withMargin(knobMargin));
    flexBox.items.add(juce::FlexItem(highKnob).withWidth(kWidth).withHeight(kHeight).withMargin(knobMargin));
    flexBox.items.add(juce::FlexItem(masterKnob).withWidth(kWidth).withHeight(kHeight).withMargin(knobMargin));

    flexBox.performLayout(knobPanelArea);

    // 2. SIGNAL CHAIN AREA
    auto chainContent = signalChainArea.withTrimmedTop(35).reduced(15);

    int pedalGap = 15;
    int singlePedalWidth = (chainContent.getWidth() - pedalGap) / 2;

    auto compArea = chainContent.removeFromLeft(singlePedalWidth);
    chainContent.removeFromLeft(pedalGap);
    auto odArea = chainContent;

    compressorBox.setBounds(compArea);
    overdriveBox.setBounds(odArea);

    // Comp
    juce::FlexBox compFlex;
    compFlex.flexDirection = juce::FlexBox::Direction::row;
    compFlex.justifyContent = juce::FlexBox::JustifyContent::center;
    compFlex.alignItems = juce::FlexBox::AlignItems::center;

    int cWidth = 48;
    int cHeight = 78;
    juce::FlexItem::Margin cMargin(0, 2, 0, 2);

    compFlex.items.add(juce::FlexItem(compSustainKnob).withWidth(cWidth).withHeight(cHeight).withMargin(cMargin));
    compFlex.items.add(juce::FlexItem(compAttackKnob).withWidth(cWidth).withHeight(cHeight).withMargin(cMargin));
    compFlex.items.add(juce::FlexItem(compBlendKnob).withWidth(cWidth).withHeight(cHeight).withMargin(cMargin));
    compFlex.items.add(juce::FlexItem(compLevelKnob).withWidth(cWidth).withHeight(cHeight).withMargin(cMargin));

    compFlex.performLayout(compArea.withTrimmedTop(25));

    // OD
    juce::FlexBox odFlex;
    odFlex.flexDirection = juce::FlexBox::Direction::row;
    odFlex.justifyContent = juce::FlexBox::JustifyContent::center;
    odFlex.alignItems = juce::FlexBox::AlignItems::center;

    int odWidth = 54;
    int odHeight = 82;
    juce::FlexItem::Margin odMargin(0, 4, 0, 4);

    odFlex.items.add(juce::FlexItem(odGainKnob).withWidth(odWidth).withHeight(odHeight).withMargin(odMargin));
    odFlex.items.add(juce::FlexItem(odToneKnob).withWidth(odWidth).withHeight(odHeight).withMargin(odMargin));
    odFlex.items.add(juce::FlexItem(odLevelKnob).withWidth(odWidth).withHeight(odHeight).withMargin(odMargin));

    odFlex.performLayout(odArea.withTrimmedTop(25));


    // 3. CAB SIM AREA
    auto cabContentArea = cabSimArea.withTrimmedTop(40).reduced(15);

    loadIRButton.setBounds(cabContentArea.removeFromTop(30));
}

void NeuralFlowAmpAudioProcessorEditor::timerCallback()
{
    if (audioProcessor.isNextFrameReady.load())
    {
        audioProcessor.isNextFrameReady.store(false);

        scopeDataToDraw = audioProcessor.scopeData;

        repaint(waveformArea);
    }
}