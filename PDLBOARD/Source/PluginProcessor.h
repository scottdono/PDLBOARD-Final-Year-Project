/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GuitarEffects.h"

//==============================================================================
/**
*/
class PDLBOARDAudioProcessor  : public foleys::MagicProcessor
{
public:
    //==============================================================================
    PDLBOARDAudioProcessor();
    ~PDLBOARDAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    double getTailLengthSeconds() const override;

    // Personal functions.
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    float lin_interp(float sample_x, float sample_x1, float inPhase);

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState treeState;

    // Circular buffer data
    float* mCircularBufferLeft;
    float* mCircularBufferRight;

    int mCircularBufferLength;
    int mCircularBufferWriteHead;

    float mFeedbackLeft;
    float mFeedbackRight;

    // LFO data
    float mLFOPhase;

    float mDelayTimeInSamples;
    float mDelayReadHead;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PDLBOARDAudioProcessor)
};
