#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NeuralFlowAmpAudioProcessorEditor::NeuralFlowAmpAudioProcessorEditor (NeuralFlowAmpAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (1600, 900);

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

    setupKnob(driveKnob, driveAttachment, "DRIVE", "DRIVE", juce::Colour(255, 140, 0));
    setupKnob(lowKnob, lowAttachment, "LOW", "LOW", juce::Colour(0, 213, 255));
    setupKnob(midKnob, midAttachment, "MID", "MID", juce::Colour(0, 213, 255));
    setupKnob(highKnob, highAttachment, "HIGH", "HIGH", juce::Colour(0, 213, 255));
    setupKnob(masterKnob, masterAttachment, "MASTER", "MASTER", juce::Colour(0, 255, 150));
    startTimer(60); 
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