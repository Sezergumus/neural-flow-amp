#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        int knobHeight = height - 20;
        auto radius = (float)juce::jmin(width / 2, knobHeight / 2) - 6.0f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)knobHeight * 0.5f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        g.setColour(juce::Colour(35, 38, 42));
        g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

        g.setColour(juce::Colour(10, 10, 10)); 
        g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 1.5f);

        juce::Path arcPath;
        arcPath.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);

        auto neonColour = slider.findColour(juce::Slider::rotarySliderFillColourId);

        for (float thickness = 10.0f; thickness > 1.0f; thickness -= 2.0f)
        {
            float alpha = juce::jmap(thickness, 1.0f, 10.0f, 0.6f, 0.05f);
            g.setColour(neonColour.withAlpha(alpha));
            g.strokePath(arcPath, juce::PathStrokeType(thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        g.setColour(neonColour.brighter(0.8f));
        g.strokePath(arcPath, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path pointer;
        pointer.addRectangle(-1.5f, -radius + 4.0f, 3.0f, radius * 0.4f);
        pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.fillPath(pointer);

        g.setColour(juce::Colour(255,255,255));
        g.setFont(juce::Font(16.0f, juce::Font::bold));
        g.drawText(slider.getName(), x, y + knobHeight, width, 20, juce::Justification::centred);
    }

    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        auto bounds = label.getLocalBounds().toFloat();

        g.setColour(juce::Colour(10, 10, 10));
        g.fillRoundedRectangle(bounds, 4.0f);

        g.setColour(juce::Colour(45, 45, 45));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        auto textColour = label.findColour(juce::Label::textColourId);
        g.setColour(textColour);
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText(label.getText(), label.getLocalBounds(), juce::Justification::centred, false);
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
