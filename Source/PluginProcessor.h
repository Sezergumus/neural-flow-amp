#pragma once

#include <JuceHeader.h>
#include "Crunch.h"

//==============================================================================
/**
*/
class NeuralFlowAmpAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NeuralFlowAmpAudioProcessor();
    ~NeuralFlowAmpAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    static constexpr int scopeSize = 512;
    std::array<float, scopeSize> scopeData;
    std::atomic<bool> isNextFrameReady{ false };

    void loadImpulseResponse(const juce::File& file);

    juce::dsp::Convolution cabSimulator;
    juce::AudioFormatManager formatManager;
private:
    //==============================================================================    
	juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    Crunch crunchEffect;

	using Filter = juce::dsp::IIR::Filter<float>;
	juce::dsp::ProcessorChain<Filter, Filter, Filter> eqChain;

    int scopeDataIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeuralFlowAmpAudioProcessor)
};
