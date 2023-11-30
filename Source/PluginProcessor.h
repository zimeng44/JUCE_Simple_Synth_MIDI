/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MIDISynth.h"

//==============================================================================
/**
*/
class MIDISynthZiMengAudioProcessor  : public juce::AudioProcessor/*, public juce::AudioProcessorValueTreeState::Listener*/
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    MIDISynthZiMengAudioProcessor();
    ~MIDISynthZiMengAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
//    void parameterChanged (const juce::String &parameterID, float newValue) override;
    
    AudioBufferQueue<float>& getAudioBufferQueue() noexcept        { return audioBufferQueue; }
    
    juce::MidiMessageCollector& getMidiMessageCollector() noexcept { return midiMessageCollector; }
    
    std::unique_ptr<ToneBank> toneBank;
    bool testing;
    double phase = 0.0;
    
    juce::AudioProcessorValueTreeState vts;

private:
    juce::MidiMessageCollector midiMessageCollector;
    AudioBufferQueue<float> audioBufferQueue;
    ScopeDataCollector<float> scopeDataCollector { audioBufferQueue };
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MIDISynthZiMengAudioProcessor)
};
