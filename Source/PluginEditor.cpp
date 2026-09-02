/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NeuralFlowAmpAudioProcessorEditor::NeuralFlowAmpAudioProcessorEditor (NeuralFlowAmpAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

	driveKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
	driveKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
	addAndMakeVisible(driveKnob);

	// Create attachment between slider and APVTS parameter
	driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.apvts, "DRIVE", driveKnob);
}

NeuralFlowAmpAudioProcessorEditor::~NeuralFlowAmpAudioProcessorEditor()
{
}

//==============================================================================
void NeuralFlowAmpAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
}

void NeuralFlowAmpAudioProcessorEditor::resized()
{
    const int knobSize = 100;
    driveKnob.setBounds((getWidth() - knobSize) / 2,
                        (getHeight() - knobSize) / 2,
                        knobSize, knobSize);
}
