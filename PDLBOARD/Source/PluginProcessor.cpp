/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PDLBOARDAudioProcessor::createParameterLayout() 
{
    /*
    * This function is used to call the various 'add parameter' functions from the GuitarEffects file.
    * It defines the IDs, Names, Ranges and Values for the parameters as well as groups them together.
    * These groups can then be added to a layout and easily edited with the PluginGuiMagic module.
    * 
    */

    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    GuitarEffectAudioProcessor::addODParameters(layout);
    GuitarEffectAudioProcessor::addChorusParameters(layout);
    GuitarEffectAudioProcessor::addDelayParameters(layout);
    return layout;
}

float PDLBOARDAudioProcessor::lin_interp(float sample_x, float sample_x1, float inPhase)
{
    /*
    * This function provides the formula for linear interpolation which allows for smooth changes during playback.
    */

    //Wikipedia formula
    return (1 - inPhase) * sample_x + inPhase * sample_x1;
}

//==============================================================================
PDLBOARDAudioProcessor::PDLBOARDAudioProcessor()
: treeState(*this, nullptr, ProjectInfo::projectName, createParameterLayout())
{
    // Load the GUI theme from the xml in the resources.
    magicState.setGuiValueTree(BinaryData::theme_copy_xml, BinaryData::theme_copy_xmlSize);

    // Call functions to push effect parameters into the treeState.
    GuitarEffectAudioProcessor::Overdrive::Overdrive(treeState);
    GuitarEffectAudioProcessor::Chorus::Chorus(treeState);
    GuitarEffectAudioProcessor::Delay::Delay(treeState);

    // Initialise data to default values.
    mCircularBufferLeft = nullptr;
    mCircularBufferRight = nullptr;
    mCircularBufferWriteHead = 0;
    mCircularBufferLength = 0;

    mFeedbackLeft = 0;
    mFeedbackRight = 0;

    mDelayTimeInSamples = 0;
    mDelayReadHead = 0;

    mLFOPhase = 0;
}

PDLBOARDAudioProcessor::~PDLBOARDAudioProcessor()
{
}

//==============================================================================

double PDLBOARDAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================
void PDLBOARDAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    /*
    * The prepareToPlay function is called to intitialise anything that will be needed before the main process beings.
    */

    // Initialise the phase;
    mLFOPhase = 0;

    // Initialise the writehead
    mCircularBufferWriteHead = 0;

    // Calculate the circular buffer length
    mCircularBufferLength = sampleRate * MAX_DELAY_TIME;

    // Initialise the left buffer
    if (mCircularBufferLeft == nullptr) {
        mCircularBufferLeft = new float[mCircularBufferLength];
    }

    // Clear any junk data in the buffer
    juce::zeromem(mCircularBufferLeft, mCircularBufferLength * sizeof(float));

    // Same for the right buffer
    if (mCircularBufferRight == nullptr) {
        mCircularBufferRight = new float[mCircularBufferLength];
    }

    juce::zeromem(mCircularBufferRight, mCircularBufferLength * sizeof(float));
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
    /*
    * The process block is the main block of code where the audio signals 
    * are processed and where the digital signal processing can occur.
    */

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Get parameters for overdrive.
    auto drive = treeState.getRawParameterValue("overdrive");
    auto range = treeState.getRawParameterValue("range");
    auto blend = treeState.getRawParameterValue("blend");
    auto volume = treeState.getRawParameterValue("volume");
    auto overdriveOnOff = treeState.getParameterAsValue("onoff1");

    // Get parameters for chorus.
    auto cDryWet = treeState.getRawParameterValue("dry/wet1");
    auto cDepth = treeState.getRawParameterValue("depth");
    auto cRate = treeState.getRawParameterValue("rate");
    auto cOffset = treeState.getRawParameterValue("offset");
    auto cFeedback = treeState.getRawParameterValue("feedback1");
    auto cType = treeState.getRawParameterValue("type");
    auto chorusOnOff = treeState.getParameterAsValue("onoff2");

    // Get parameters for delay.
    auto dDryWet = treeState.getRawParameterValue("dry/wet2");
    auto dFeedback = treeState.getRawParameterValue("feedback2");
    auto dDelayTime = treeState.getRawParameterValue("delaytime");
    auto delayOnOff = treeState.getParameterAsValue("onoff3");

    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Iterate through the audio channels.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        // Obtain the audiodata pointers for single channel for distortion algorithm
        float* channelData = buffer.getWritePointer(channel);

        // Obtain the audio data pointers for left and right channel for delay effects
        float* leftChannel = buffer.getWritePointer(0);
        float* rightChannel = buffer.getWritePointer(1);

        // Set the delay time based on sample rate and delay parameter
        mDelayTimeInSamples = getSampleRate() * *dDelayTime;
        
        // Iterate through the audio buffer
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            // Process Overdrive if button is turned on.
            if (overdriveOnOff == true)
            {
                auto cleanSignal = *channelData;

                *channelData *= *drive * *range;

                *channelData = (((((2.f / juce::float_Pi) * atan(*channelData)) * *blend) + (cleanSignal * (1.f - *blend))) / 2) * *volume;

                channelData++;
            }

            //===========================================================================

            // Process Chorus if button is on.
            if (chorusOnOff == true)
            {
                // Write into the circular buffer
                mCircularBufferLeft[mCircularBufferWriteHead] = leftChannel[i] + mFeedbackLeft;
                mCircularBufferRight[mCircularBufferWriteHead] = rightChannel[i] + mFeedbackRight;

                // Generate the left LFO output. LFO = Low Frequency Oscillator which is used to manipulate the waveform
                float lfoOutLeft = sin(2 * juce::MathConstants<float>::pi * mLFOPhase);

                // Calculate the right channel lfo phase
                float lfoPhaseRight = mLFOPhase + *cOffset;

                // Check the phase is not greater than 1
                if (lfoPhaseRight > 1)
                {
                    lfoPhaseRight -= 1;
                }

                // Generate the left LFO output
                float lfoOutRight = sin(2 * juce::MathConstants<float>::pi * lfoPhaseRight);

                // Moving the LFO phase forward
                mLFOPhase += *cRate / getSampleRate();

                if (mLFOPhase > 1)
                {
                    mLFOPhase -= 1;
                }

                // Control depth of LFO by multiplying by the depth parameter which is attatched to the depth slider
                lfoOutLeft *= *cDepth;
                lfoOutRight *= *cDepth;

                float lfoOutMappedLeft = 0;
                float lfoOutMappedRight = 0;

                // Map the LFO output to our desired delay times. 

                // Chorus
                if (*cType == 0)
                {
                    lfoOutMappedLeft = juce::jmap(lfoOutLeft, -1.f, 1.f, 0.005f, 0.03f);
                    lfoOutMappedRight = juce::jmap(lfoOutRight, -1.f, 1.f, 0.005f, 0.03f);
                }
                else //Flanger
                {
                    lfoOutMappedLeft = juce::jmap(lfoOutLeft, -1.f, 1.f, 0.001f, 0.005f);
                    lfoOutMappedRight = juce::jmap(lfoOutRight, -1.f, 1.f, 0.001f, 0.005f);
                }

                // Calculate the delay lengths and samples for whatever delay times are chosen. i.e. Chorus or flanger
                float delayTimeSamplesLeft = getSampleRate() * lfoOutMappedLeft;
                float delayTimeSamplesRight = getSampleRate() * lfoOutMappedRight;

                // Calculate the left read head position
                float delayReadHeadLeft = mCircularBufferWriteHead - delayTimeSamplesLeft;

                if (delayReadHeadLeft < 0) {
                    delayReadHeadLeft += mCircularBufferLength;
                }

                // Calculate the right read head position
                float delayReadHeadRight = mCircularBufferWriteHead - delayTimeSamplesRight;

                if (delayReadHeadRight < 0) {
                    delayReadHeadRight += mCircularBufferLength;
                }

                // Calculate the linear interpolation points for the left channel for smooth parameter changes
                int readHeadLeft_x = (int)delayReadHeadLeft;
                int readHeadLeft_x1 = readHeadLeft_x + 1;
                float readHeadFloatLeft = delayReadHeadLeft - readHeadLeft_x;

                if (readHeadLeft_x1 >= mCircularBufferLength)
                {
                    readHeadLeft_x1 -= mCircularBufferLength;
                }

                // Same for right channel
                int readHeadRight_x = (int)delayReadHeadRight;
                int readHeadRight_x1 = readHeadRight_x + 1;
                float readHeadFloatRight = delayReadHeadRight - readHeadRight_x;

                if (readHeadRight_x1 >= mCircularBufferLength)
                {
                    readHeadRight_x1 -= mCircularBufferLength;
                }

                // Generate the output samples. See lin_interp() at the top of the code.
                float delay_sample_left = lin_interp(mCircularBufferLeft[readHeadLeft_x], mCircularBufferLeft[readHeadLeft_x1], readHeadFloatLeft);
                float delay_sample_right = lin_interp(mCircularBufferRight[readHeadRight_x], mCircularBufferRight[readHeadRight_x1], readHeadFloatRight);

                // Feedback from output that can be modified using the sliders that is then added to the start of the circular buffer at the start of the process block
                mFeedbackLeft = delay_sample_left * *cFeedback;
                mFeedbackRight = delay_sample_right * *cFeedback;

                mCircularBufferWriteHead++;

                if (mCircularBufferWriteHead >= mCircularBufferLength) {
                    mCircularBufferWriteHead = 0;
                }

                float dryAmount = 1 - *cDryWet;
                float wetAmount = *cDryWet;

                buffer.setSample(0, i, buffer.getSample(0, i) * dryAmount + delay_sample_left * wetAmount);
                buffer.setSample(1, i, buffer.getSample(1, i) * dryAmount + delay_sample_right * wetAmount);
               
            } // end if for chorus

            //===========================================================================

            // Process Dekay if button is on.
            if (delayOnOff == true)
            {
                mCircularBufferLeft[mCircularBufferWriteHead] = leftChannel[i] + mFeedbackLeft;
                mCircularBufferRight[mCircularBufferWriteHead] = rightChannel[i] + mFeedbackRight;

                mDelayReadHead = mCircularBufferWriteHead - mDelayTimeInSamples;

                if (mDelayReadHead < 0) {
                    mDelayReadHead += mCircularBufferLength;
                }

                int readHead_x = (int)mDelayReadHead;
                int readHead_x1 = readHead_x + 1;
                float readHeadFloat = mDelayReadHead - readHead_x;

                if (readHead_x1 >= mCircularBufferLength)
                {
                    readHead_x1 -= mCircularBufferLength;
                }

                float delay_sample_left = lin_interp(mCircularBufferLeft[readHead_x], mCircularBufferLeft[readHead_x1], readHeadFloat);
                float delay_sample_right = lin_interp(mCircularBufferRight[readHead_x], mCircularBufferRight[readHead_x1], readHeadFloat);

                mFeedbackLeft = delay_sample_left * *dFeedback;
                mFeedbackRight = delay_sample_right * *dFeedback;

                mCircularBufferWriteHead++;

                if (mCircularBufferWriteHead >= mCircularBufferLength) {
                    mCircularBufferWriteHead = 0;
                }

                buffer.setSample(0, i, buffer.getSample(0, i) * (1 - *dDryWet) + delay_sample_left * *dDryWet);
                buffer.setSample(1, i, buffer.getSample(1, i) * (1 - *dDryWet) + delay_sample_right * *dDryWet);  
            }// end if for delay

        }// end buffer loop
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PDLBOARDAudioProcessor();
}
