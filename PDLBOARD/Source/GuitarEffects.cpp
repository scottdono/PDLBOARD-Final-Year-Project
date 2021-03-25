/*
  ==============================================================================

    GuitarEffects.cpp
    Created: 19 Mar 2021 5:44:28pm
    Author:  Scott

  ==============================================================================
*/

#include "GuitarEffects.h"

namespace IDs
{
    // Unique IDs for effect parameters

    static juce::String overdrive_id{ "overdrive" };
    static juce::String blend_id{ "blend" };
    static juce::String range_id{ "range" };
    static juce::String volume_id{ "volume" };
    static juce::String onoff_id1{ "onoff1" };

    static juce::String chorusDryWet_id{ "dry/wet1" };
    static juce::String chorusDepth_id{ "depth" };
    static juce::String chorusRate_id{ "rate" };
    static juce::String chorusOffset_id{ "offset" };
    static juce::String chorusFeedback_id{ "feedback1" };
    static juce::String chorusType_id{ "type" };
    static juce::String onoff_id2{ "onoff2" };

    static juce::String delayDryWet_id{ "dry/wet2" };
    static juce::String delayFeedback_id{ "feedback2" };
    static juce::String delayTime_id{ "delaytime" };
    static juce::String onoff_id3{ "onoff3" };

}

void GuitarEffectAudioProcessor::addODParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto overdrive = std::make_unique<juce::AudioParameterFloat>(IDs::overdrive_id, "Overdrive", juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 0.5f);
    auto range = std::make_unique<juce::AudioParameterFloat>(IDs::range_id, "Range", juce::NormalisableRange<float>(0.f, 300.f, 0.01f), 0.5f);
    auto blend = std::make_unique<juce::AudioParameterFloat>(IDs::blend_id, "Blend", juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 0.5f);
    auto volume = std::make_unique<juce::AudioParameterFloat>(IDs::volume_id, "Volume", juce::NormalisableRange<float>(0.f, 3.f, 0.01f), 0.5f);
    auto onoff = std::make_unique<juce::AudioParameterBool>(IDs::onoff_id1, "On / Off", false);

    auto group = std::make_unique<juce::AudioProcessorParameterGroup>("distortion", "Distortion", "|",
                                                                        std::move(overdrive),
                                                                        std::move(range),
                                                                        std::move(blend),
                                                                        std::move(volume),
                                                                        std::move(onoff));
    layout.add(std::move(group));
}

void GuitarEffectAudioProcessor::addChorusParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto chorusDryWet = std::make_unique<juce::AudioParameterFloat>(IDs::chorusDryWet_id, "Dry / Wet", juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 0.5f);
    auto chorusDepth = std::make_unique<juce::AudioParameterFloat>(IDs::chorusDepth_id, "Depth", juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 0.5f);
    auto chorusRate = std::make_unique<juce::AudioParameterFloat>(IDs::chorusRate_id, "Rate", juce::NormalisableRange<float>(0.f, 10.f, 0.01f), 0.5f);
    auto chorusOffset = std::make_unique<juce::AudioParameterFloat>(IDs::chorusOffset_id, "Phase Offset", juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 0.f);
    auto chorusFeedback = std::make_unique<juce::AudioParameterFloat>(IDs::chorusFeedback_id, "Feedback", juce::NormalisableRange<float>(0.f, 0.98f, 0.01f), 0.5f);
    auto chorusType = std::make_unique<juce::AudioParameterChoice>(IDs::chorusType_id, "Type", juce::StringArray("Chorus", "Flanger"), 0);

    auto group = std::make_unique<juce::AudioProcessorParameterGroup>("chorus", "Chorus", "|",
                                                                        std::move(chorusDryWet),
                                                                        std::move(chorusDepth),
                                                                        std::move(chorusRate),
                                                                        std::move(chorusOffset),
                                                                        std::move(chorusFeedback),
                                                                        std::move(chorusType));
    layout.add(std::move(group));

}

void GuitarEffectAudioProcessor::addDelayParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto delayDryWet = std::make_unique<juce::AudioParameterFloat>(IDs::delayDryWet_id, "Dry / Wet", juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 0.5f);
    auto delayFeedback = std::make_unique<juce::AudioParameterFloat>(IDs::delayFeedback_id, "Feedback", juce::NormalisableRange<float>(0.f, 0.98f, 0.01f), 0.5f);
    auto delayTime = std::make_unique<juce::AudioParameterFloat>(IDs::delayTime_id, "Delay", juce::NormalisableRange<float>(0.f, MAX_DELAY_TIME, 0.01f), 0.5f);

    auto group = std::make_unique<juce::AudioProcessorParameterGroup>("delay", "Delay", "|",
                                                                        std::move(delayDryWet),
                                                                        std::move(delayFeedback),
                                                                        std::move(delayTime));
    layout.add(std::move(group));
}

GuitarEffectAudioProcessor::Overdrive::Overdrive (juce::AudioProcessorValueTreeState& stateToUse) : state (stateToUse)
{

    // Construct parameters for all effects into ValueTreeState
    mDriveParameter = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter(IDs::overdrive_id));
    jassert(mDriveParameter);
    mBlendParameter = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter(IDs::blend_id));
    jassert(mBlendParameter);
    mRangeParameter = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter(IDs::range_id));
    jassert(mRangeParameter);
    mVolumeParameter = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter(IDs::volume_id));
    jassert(mVolumeParameter);

}


GuitarEffectAudioProcessor::Chorus::Chorus (juce::AudioProcessorValueTreeState& stateToUse) : state(stateToUse)
{
    mChorusDryWetParameter = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter(IDs::chorusDryWet_id));
    jassert(mChorusDryWetParameter);
    mChorusDepthParameter = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter(IDs::chorusDepth_id));
    jassert(mChorusDepthParameter);
    mChorusRateParameter = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter(IDs::chorusRate_id));
    jassert(mChorusRateParameter);
    mChorusPhaseOffsetParameter = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter(IDs::chorusOffset_id));
    jassert(mChorusPhaseOffsetParameter);
    mChorusFeedbackParameter = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter(IDs::chorusFeedback_id));
    jassert(mChorusFeedbackParameter);
    mChorusTypeParameter = dynamic_cast<juce::AudioParameterChoice*>(state.getParameter(IDs::chorusType_id));
    jassert(mChorusTypeParameter);
}

GuitarEffectAudioProcessor::Delay::Delay (juce::AudioProcessorValueTreeState& stateToUse) : state(stateToUse)
{
    mDelayDryWetParameter  = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter(IDs::delayDryWet_id));
    jassert(mDelayDryWetParameter);
    mDelayFeedbackParameter = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter(IDs::delayFeedback_id));
    jassert(mDelayFeedbackParameter);
    mDelayTimeParameter = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter(IDs::delayTime_id));
    jassert(mDelayTimeParameter);
}
