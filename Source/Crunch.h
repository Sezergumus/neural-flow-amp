#pragma once
#include <JuceHeader.h>

class Crunch {
	public: 
		void prepare (double sampleRate, int samplesPerBlock);

		void process (juce::AudioBuffer<float>& buffer);

		void setDrive (float newDrive);

private: 
	float drive{ 1.0f };
	float currentSampleRate{ 44100.0 };
};
