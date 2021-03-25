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
class PDLBOARDAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PDLBOARDAudioProcessorEditor (PDLBOARDAudioProcessor&);
    ~PDLBOARDAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PDLBOARDAudioProcessor& audioProcessor;

    juce::Slider slider;
    juce::Slider dial;
    juce::TextButton powerButton;

public:
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dialAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PDLBOARDAudioProcessorEditor)
};
