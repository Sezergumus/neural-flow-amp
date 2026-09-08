/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NeuralFlowAmpAudioProcessor::NeuralFlowAmpAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
	 ), apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
	formatManager.registerBasicFormats();
}

NeuralFlowAmpAudioProcessor::~NeuralFlowAmpAudioProcessor()
{
}

//==============================================================================
const juce::String NeuralFlowAmpAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NeuralFlowAmpAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NeuralFlowAmpAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NeuralFlowAmpAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NeuralFlowAmpAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NeuralFlowAmpAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NeuralFlowAmpAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NeuralFlowAmpAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NeuralFlowAmpAudioProcessor::getProgramName (int index)
{
    return {};
}

void NeuralFlowAmpAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NeuralFlowAmpAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	crunchEffect.prepare(sampleRate, samplesPerBlock);
   
	juce::dsp::ProcessSpec spec;
	spec.sampleRate = sampleRate;
	spec.maximumBlockSize = samplesPerBlock;
	spec.numChannels = getTotalNumOutputChannels();

	eqChain.prepare(spec);

	cabSimulator.prepare(spec);
}

void NeuralFlowAmpAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NeuralFlowAmpAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NeuralFlowAmpAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

	// Clean output buffers in case of more output channels than input channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

	auto currentDrive = apvts.getRawParameterValue("DRIVE")->load();

	crunchEffect.setDrive(currentDrive);
    crunchEffect.process(buffer);

	float lowDb = juce::jmap(apvts.getRawParameterValue("LOW")->load(), 0.0f, 10.0f, -15.0f, 15.0f);
	float midDb = juce::jmap(apvts.getRawParameterValue("MID")->load(), 0.0f, 10.0f, -15.0f, 15.0f);
    float highDb = juce::jmap(apvts.getRawParameterValue("HIGH")->load(), 0.0f, 10.0f, -15.0f, 15.0f);

    *eqChain.get<0>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), 250.0f, 0.707f, juce::Decibels::decibelsToGain(lowDb));
    *eqChain.get<1>().coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), 1000.0f, 0.707f, juce::Decibels::decibelsToGain(midDb));
    *eqChain.get<2>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(getSampleRate(), 4000.0f, 0.707f, juce::Decibels::decibelsToGain(highDb));

	juce::dsp::AudioBlock<float> block(buffer);
	juce::dsp::ProcessContextReplacing<float> context(block);
	eqChain.process(context);

	cabSimulator.process(context);

	auto currentMaster = apvts.getRawParameterValue("MASTER")->load();
	float masterGain = currentMaster / 10.0f;
	buffer.applyGain(masterGain);

    // OSCILLOSCOPE DATA
    auto* channelData = buffer.getReadPointer(0);
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        // Turn soundwaves into array
        if (scopeDataIndex < scopeSize)
        {
            scopeData[scopeDataIndex] = channelData[sample];
            scopeDataIndex++;
        }

		// When the array is full, set the flag to true and reset the index to 0
        if (scopeDataIndex >= scopeSize)
        {
            isNextFrameReady = true;
            scopeDataIndex = 0; 
        }
    }
}

//==============================================================================
bool NeuralFlowAmpAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NeuralFlowAmpAudioProcessor::createEditor()
{
    return new NeuralFlowAmpAudioProcessorEditor (*this);
}

//==============================================================================
void NeuralFlowAmpAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NeuralFlowAmpAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NeuralFlowAmpAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout NeuralFlowAmpAudioProcessor::createParameterLayout() {
	std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("DRIVE", "Drive", 1.0f, 100.0f, 50.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("LOW", "Low", 0.0f, 10.0f, 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MID", "Mid", 0.0f, 10.0f, 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("HIGH", "High", 0.0f, 10.0f, 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MASTER", "Master", 0.0f, 10.0f, 5.0f));

    return { params.begin(), params.end() };
}

void NeuralFlowAmpAudioProcessor::loadImpulseResponse(const juce::File& file) {
	if (file.existsAsFile()) {
		cabSimulator.loadImpulseResponse(file, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::yes, 0, juce::dsp::Convolution::Normalise::yes);
	}
}