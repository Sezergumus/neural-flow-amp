#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto radius = (float)juce::jmin(width / 2, height / 2) - 15.0f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;

        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        g.setColour(juce::Colour(20, 20, 20));
        g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

        juce::Path arcPath;
        arcPath.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);

        auto neonColour = juce::Colour(0, 213, 255);

        for (float thickness = 14.0f; thickness > 2.0f; thickness -= 3.0f)
        {
            float alpha = juce::jmap(thickness, 2.0f, 14.0f, 0.6f, 0.02f);
            g.setColour(neonColour.withAlpha(alpha));
            g.strokePath(arcPath, juce::PathStrokeType(thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        g.setColour(neonColour.brighter(0.8f));
        g.strokePath(arcPath, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path pointer;
        auto pointerLength = radius * 0.4f;
        pointer.addRectangle(-1.5f, -radius + 6.0f, 3.0f, pointerLength);
        pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.fillPath(pointer);
    }
};

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
    CustomLookAndFeel customTheme;
    juce::Slider driveKnob, lowKnob, midKnob, highKnob, masterKnob;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment, lowAttachment, midAttachment, highAttachment, masterAttachment;

    NeuralFlowAmpAudioProcessor& audioProcessor;

    juce::Rectangle<int> leadChannelArea;
    juce::Rectangle<int> signalChainArea;
    juce::Rectangle<int> cabSimArea;

    juce::Rectangle<int> waveformArea;
    juce::Rectangle<int> knobPanelArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeuralFlowAmpAudioProcessorEditor)
};
