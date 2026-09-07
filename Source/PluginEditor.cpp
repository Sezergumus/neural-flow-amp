#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NeuralFlowAmpAudioProcessorEditor::NeuralFlowAmpAudioProcessorEditor (NeuralFlowAmpAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (1920, 1080);

	juce::Slider* sliders[] = { &driveKnob, &lowKnob, &midKnob, &highKnob, &masterKnob };

    for (auto* slider : sliders){
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        slider->setLookAndFeel(&customTheme);
        addAndMakeVisible(slider);
    }

	// Create attachment between slider and APVTS parameter
	driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "DRIVE", driveKnob);
	lowAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "LOW", lowKnob);
	midAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "MID", midKnob);
	highAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "HIGH", highKnob);
	masterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "MASTER", masterKnob);
}

NeuralFlowAmpAudioProcessorEditor::~NeuralFlowAmpAudioProcessorEditor()
{
	driveKnob.setLookAndFeel(nullptr);
    lowKnob.setLookAndFeel(nullptr);
	midKnob.setLookAndFeel(nullptr);
	highKnob.setLookAndFeel(nullptr);
	masterKnob.setLookAndFeel(nullptr);
}

//==============================================================================
void NeuralFlowAmpAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour(12, 12, 12)); 

    auto drawPanel = [&](juce::Rectangle<int> bounds, const juce::String& title, bool isInner = false) {
        g.setColour(isInner ? juce::Colour(8, 8, 8) : juce::Colour(22, 22, 22));
        g.fillRoundedRectangle(bounds.toFloat(), 6.0f);

        g.setColour(isInner ? juce::Colour(35, 35, 35) : juce::Colour(45, 45, 45));
        g.drawRoundedRectangle(bounds.toFloat(), 6.0f, 1.2f);

        if (title.isNotEmpty() && !isInner) {
            g.setColour(juce::Colours::white.withAlpha(0.8f));
            g.setFont(juce::Font(14.0f, juce::Font::bold));
            g.drawText(title, bounds.withTrimmedLeft(15).withTrimmedTop(12).withHeight(20), juce::Justification::topLeft);
        }
        };

    drawPanel(leadChannelArea, "LEAD CHANNEL");
    drawPanel(signalChainArea, "SIGNAL CHAIN");
    drawPanel(cabSimArea, "CAB SIMULATION");
    drawPanel(waveformArea, "", true);
    drawPanel(knobPanelArea, "", true);
}

void NeuralFlowAmpAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(15);

    leadChannelArea = area.removeFromTop(area.getHeight() / 2 - 5);
    area.removeFromTop(10);

    signalChainArea = area.removeFromLeft(area.getWidth() / 2 - 5);
    area.removeFromLeft(10); 
    cabSimArea = area;

    auto leadContent = leadChannelArea.withTrimmedTop(35).reduced(15, 15);

    knobPanelArea = leadContent.removeFromRight(350);
    leadContent.removeFromRight(15); 
    waveformArea = leadContent;

    juce::FlexBox flexBox;
    flexBox.flexDirection = juce::FlexBox::Direction::row;
    flexBox.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;

    int kSize = 65;
    flexBox.items.add(juce::FlexItem(driveKnob).withWidth(kSize).withHeight(kSize));
    flexBox.items.add(juce::FlexItem(lowKnob).withWidth(kSize).withHeight(kSize));
    flexBox.items.add(juce::FlexItem(midKnob).withWidth(kSize).withHeight(kSize));
    flexBox.items.add(juce::FlexItem(highKnob).withWidth(kSize).withHeight(kSize));
    flexBox.items.add(juce::FlexItem(masterKnob).withWidth(kSize).withHeight(kSize));

    flexBox.performLayout(knobPanelArea.reduced(10));
}
