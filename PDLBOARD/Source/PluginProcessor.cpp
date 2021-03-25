/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PDLBOARDAudioProcessor::createParameterLayout() {

    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    GuitarEffectAudioProcessor::addODParameters(layout);
    GuitarEffectAudioProcessor::addChorusParameters(layout);
    GuitarEffectAudioProcessor::addDelayParameters(layout);
    return layout;


    /*
    // Store params in a vector
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;

    // define them and push them into the vector
    params.push_back(std::make_unique<juce::AudioParameterFloat>("SLIDER", "Slider", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterInt>("DIAL", "Dial", 0, 10, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("CHOICE", "Choice", juce::StringArray("Choice1", "Choice2", "Choice3"), 0));


    //auto mChorusDryWetParameter = std::make_unique<juce::AudioParameterFloat>("dry/wet", "Dry/Wet", 0.f, 1.f, 0.5f);
    //params.push_back(std::move(mChorusDryWetParameter));

    // Return them to the treeState constructor
    return { params.begin(), params.end() };
    */
}

//==============================================================================
PDLBOARDAudioProcessor::PDLBOARDAudioProcessor()
: treeState(*this, nullptr, ProjectInfo::projectName, createParameterLayout())
{
    //auto defaultGUI = magicState.createDefaultGUITree();
    //magicState.setGuiValueTree (defaultGUI);
    //GuitarEffectAudioProcessor::Overdrive:: ODParams(new GuitarEffectAudioProcessor::Overdrive (treeState));

    new GuitarEffectAudioProcessor::Overdrive(treeState);
    //new GuitarEffectAudioProcessor::Chorus(treeState);
    //new GuitarEffectAudioProcessor::Delay(treeState);


}

PDLBOARDAudioProcessor::~PDLBOARDAudioProcessor()
{
}

//==============================================================================

const juce::String PDLBOARDAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PDLBOARDAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PDLBOARDAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PDLBOARDAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PDLBOARDAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PDLBOARDAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PDLBOARDAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PDLBOARDAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PDLBOARDAudioProcessor::getProgramName (int index)
{
    return {};
}

void PDLBOARDAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PDLBOARDAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void PDLBOARDAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PDLBOARDAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

void PDLBOARDAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    auto drive = treeState.getRawParameterValue("overdrive");
    auto range = treeState.getRawParameterValue("range");
    auto blend = treeState.getRawParameterValue("blend");
    auto volume = treeState.getRawParameterValue("volume");
    bool overdriveOnOff = treeState.getRawParameterValue("onoff1");
    //DBG(*overdriveOnOff);
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            if (overdriveOnOff == true)
            {
                auto cleanSignal = *channelData;

                *channelData *= *drive * *range;

                *channelData = (((((2.f / juce::float_Pi) * atan(*channelData)) * *blend) + (cleanSignal * (1.f - *blend))) / 2) * *volume;

                channelData++;
            }
        }
    }

    /*
    // How to use param values
    auto sliderValue = treeState.getRawParameterValue("SLIDER");
    auto dialValue = treeState.getRawParameterValue("DIAL");

    DBG(*dialValue);
    */
}

//==============================================================================
bool PDLBOARDAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}


juce::AudioProcessorEditor* PDLBOARDAudioProcessor::createEditor()
{
    return new foleys::MagicPluginEditor(magicState);
    //return new PDLBOARDAudioProcessorEditor (*this);
}
/*
//==============================================================================
void PDLBOARDAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PDLBOARDAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}
*/


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PDLBOARDAudioProcessor();
}
