/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PDLBOARDAudioProcessorEditor::PDLBOARDAudioProcessorEditor (PDLBOARDAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 800);

    slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 50, 10);
    slider.setRange(0, 1.0, 0.1);
    addAndMakeVisible(slider);

    //sliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.treeState, "SLIDER", slider);

    //========================================================================================

    dial.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    dial.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 50, 10);
    dial.setRange(0, 10, 1);
    addAndMakeVisible(dial);

    //dialAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "DIAL", dial);

    //========================================================================================

    powerButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    powerButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
    powerButton.setClickingTogglesState(true);
    powerButton.onClick = [this]() {};
    addAndMakeVisible(powerButton);

    //buttonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.treeState, "CHOICE", powerButton);

}

PDLBOARDAudioProcessorEditor::~PDLBOARDAudioProcessorEditor()
{
}

//==============================================================================
void PDLBOARDAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void PDLBOARDAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    const int componentSize { 100 };

    slider.setBounds(bounds.removeFromTop(200).withSizeKeepingCentre(componentSize, componentSize));
    dial.setBounds(bounds.removeFromTop(200).withSizeKeepingCentre(componentSize, componentSize));
    powerButton.setBounds(bounds.removeFromTop(200).withSizeKeepingCentre(componentSize, componentSize));
}
