/*
  ==============================================================================

    GuitarEffects.h
    Created: 19 Mar 2021 5:44:41pm
    Author:  Scott

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define MAX_DELAY_TIME 2


class GuitarEffectAudioProcessor : public juce::AudioProcessor
{
public:
    static void addODParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    static void addDelayParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    static void addChorusParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);

    float lin_interp(float sample_x, float sample_x1, float inPhase);

    GuitarEffectAudioProcessor() = default;

    class Overdrive 
    {
    public:
        Overdrive (juce::AudioProcessorValueTreeState& state);

    private:
        juce::AudioProcessorValueTreeState& state;
        juce::AudioParameterFloat* mDriveParameter = nullptr;
        juce::AudioParameterFloat* mRangeParameter = nullptr;
        juce::AudioParameterFloat* mBlendParameter = nullptr;
        juce::AudioParameterFloat* mVolumeParameter = nullptr;
        juce::AudioParameterBool* mOverdriveOnOff = nullptr;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Overdrive);
    };

    class Chorus
    {
    public:
        Chorus (juce::AudioProcessorValueTreeState& state);

    private:
        // Parameter declarations
        juce::AudioProcessorValueTreeState& state;
        juce::AudioParameterFloat* mChorusDryWetParameter = nullptr;
        juce::AudioParameterFloat* mChorusDepthParameter = nullptr;
        juce::AudioParameterFloat* mChorusRateParameter = nullptr;
        juce::AudioParameterFloat* mChorusPhaseOffsetParameter = nullptr;
        juce::AudioParameterFloat* mChorusFeedbackParameter = nullptr;
        juce::AudioParameterChoice* mChorusTypeParameter = nullptr;


        // Circular buffer data
        float* mCircularBufferLeft;
        float* mCircularBufferRight;

        int mCircularBufferLength;
        int mCircularBufferWriteHead;

        float mFeedbackLeft;
        float mFeedbackRight;

        // LFO data
        float mLFOPhase;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Chorus)
    };

    class Delay
    {
    public:
        Delay(juce::AudioProcessorValueTreeState& state);

    private:
        juce::AudioProcessorValueTreeState& state;
        juce::AudioParameterFloat* mDelayDryWetParameter = nullptr;
        juce::AudioParameterFloat* mDelayFeedbackParameter = nullptr;
        juce::AudioParameterFloat* mDelayTimeParameter = nullptr;

        float* mCircularBufferLeft;
        float* mCircularBufferRight;

        int mCircularBufferLength;
        int mCircularBufferWriteHead;

        float mFeedbackLeft;
        float mFeedbackRight;

        float mDelayTimeInSamples;
        float mDelayReadHead;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Delay)
    };

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuitarEffectAudioProcessor)
};
