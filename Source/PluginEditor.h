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
class MIDISynthZiMengAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Button::Listener, private juce::Timer
{
public:
    MIDISynthZiMengAudioProcessorEditor (MIDISynthZiMengAudioProcessor&/*, juce::AudioProcessorValueTreeState& vts*/);
    ~MIDISynthZiMengAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    void buttonToggle(Tone::WaveType w);
    void timerCallback() override;
    
    juce::TextButton sinButton;
    juce::TextButton stButton;
    juce::TextButton sqButton;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    MIDISynthZiMengAudioProcessor& audioProcessor;
    
    juce::MidiKeyboardState midiKeyboardState;
    juce::MidiKeyboardComponent midiKeyboardComponent { 
        midiKeyboardState, juce::MidiKeyboardComponent::horizontalKeyboard };
    
    ScopeComponent<float> scopeComponent;
    
    Tone::WaveType waveType;
    
//    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> sinButtonAttachment;
//    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> stButtonAttachment;
//    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> sqButtonAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MIDISynthZiMengAudioProcessorEditor)
};
