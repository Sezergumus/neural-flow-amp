#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NeuralFlowAmpAudioProcessorEditor::NeuralFlowAmpAudioProcessorEditor (NeuralFlowAmpAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (900, 600);

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
	g.fillAll(juce::Colour(15, 15, 15));
}

void NeuralFlowAmpAudioProcessorEditor::resized()
{
    auto topArea = getLocalBounds().removeFromTop(200).reduced(20);

	juce::FlexBox flexBox;
	flexBox.flexDirection = juce::FlexBox::Direction::row;
	flexBox.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    flexBox.alignItems = juce::FlexBox::AlignItems::center;

    flexBox.items.add(juce::FlexItem(driveKnob).withWidth(100).withHeight(100));
    flexBox.items.add(juce::FlexItem(lowKnob).withWidth(90).withHeight(90)); 
    flexBox.items.add(juce::FlexItem(midKnob).withWidth(90).withHeight(90));
    flexBox.items.add(juce::FlexItem(highKnob).withWidth(90).withHeight(90));
    flexBox.items.add(juce::FlexItem(masterKnob).withWidth(100).withHeight(100));

    flexBox.performLayout(topArea);
}
