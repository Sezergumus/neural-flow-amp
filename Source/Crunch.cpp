#include "Crunch.h"

void Crunch::prepare(double sampleRate, int samplesPerBlock) {
	currentSampleRate = sampleRate;
}

void Crunch::process(juce::AudioBuffer<float>& buffer) {
	for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
		auto* channelData = buffer.getWritePointer(channel);
		for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
			
			// Apply drive to the input signal
			float input = channelData[sample] * drive;

			// Apply a simple soft clipping algorithm
			float output = std::tanh(input);

			// Write the processed sample back to the buffer
			channelData[sample] = output;
		}
	}
}

void  Crunch::setDrive(float newDrive) {
	drive = newDrive;
}