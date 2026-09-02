/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class NeuralFlowAmpAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NeuralFlowAmpAudioProcessorEditor (NeuralFlowAmpAudioProcessor&);
    ~NeuralFlowAmpAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    juce::Slider driveKnob;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;

    NeuralFlowAmpAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeuralFlowAmpAudioProcessorEditor)
};
