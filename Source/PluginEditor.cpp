/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MIDISynthZiMengAudioProcessorEditor::MIDISynthZiMengAudioProcessorEditor (MIDISynthZiMengAudioProcessor& p/*, juce::AudioProcessorValueTreeState& vts*/)
    : AudioProcessorEditor (&p), valueTreeState (p.vts), audioProcessor (p), scopeComponent (p.getAudioBufferQueue())
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 250);
    auto area = getLocalBounds();
    
    addAndMakeVisible(sinButton);
    sinButton.addListener(this);
    sinButton.setEnabled(false);
    
    addAndMakeVisible(stButton);
    stButton.addListener(this);
    
    addAndMakeVisible(sqButton);
    sqButton.addListener(this);
    
    addAndMakeVisible (scopeComponent);
    
    scopeComponent.setTopLeftPosition (0, 0);
    scopeComponent.setSize (area.getWidth(), area.getHeight()/2.5);
    
    addAndMakeVisible (midiKeyboardComponent);
    midiKeyboardComponent.setMidiChannel (2);
    midiKeyboardComponent.setVelocity(0.7f, true);
    midiKeyboardState.addListener (&audioProcessor.getMidiMessageCollector());
    
//    waveType = audioProcessor.toneBank->getWaveType();
    
    startTimerHz (10);
    
//    sinButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(valueTreeState, "sinButton", sinButton);
//    stButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(valueTreeState, "stEnabled", stButton);
//    sqButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(valueTreeState, "sqEnabled", sqButton);
}

MIDISynthZiMengAudioProcessorEditor::~MIDISynthZiMengAudioProcessorEditor()
{
    sinButton.removeListener(this);
    stButton.removeListener(this);
    sqButton.removeListener(this);
    midiKeyboardState.removeListener (&audioProcessor.getMidiMessageCollector());
}

//==============================================================================
void MIDISynthZiMengAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
//    g.drawFittedText ("", getLocalBounds(), juce::Justification::centred, 1);
}

void MIDISynthZiMengAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto area = getLocalBounds();
    
    sinButton.setBounds(5, area.getHeight()/2.2, 60, 30);
    sinButton.setButtonText("Sine");
    
    stButton.setBounds(70, area.getHeight()/2.2, 70, 30);
    stButton.setButtonText("Sawtooth");
    
    sqButton.setBounds(145, area.getHeight()/2.2, 60, 30);
    sqButton.setButtonText("Square");
    
    midiKeyboardComponent.setBounds (0, area.getHeight()*0.65, area.getWidth(), area.getHeight()*0.35);
}

void MIDISynthZiMengAudioProcessorEditor::buttonClicked(juce::Button* button) {
    
    if (button==&sinButton) {
        buttonToggle(Tone::Sine);
        return;
    }
    
    if (button==&stButton) {
        buttonToggle(Tone::Sawtooth);
        return;
    }
    
    if (button==&sqButton) {
        buttonToggle(Tone::Square);
        return;
    }
    
//    if(audioProcessor.testing == true){
//        audioProcessor.testing = false;
//    }else {
//        audioProcessor.testing = true;
//    }
}

void MIDISynthZiMengAudioProcessorEditor::buttonToggle(Tone::WaveType newType){
    waveType = newType;
    
    if(audioProcessor.toneBank->getWaveType() != newType){
        
        audioProcessor.toneBank->setWaveType(newType);
        
        auto state = audioProcessor.toneBank->getWaveType();
        
        switch (state) {
            case Tone::Sine:
                sinButton.setEnabled(false);
                stButton.setEnabled(true);
                sqButton.setEnabled(true);
                break;
            
            case Tone::Sawtooth:
                sinButton.setEnabled(true);
                stButton.setEnabled(false);
                sqButton.setEnabled(true);
                break;
            
            case Tone::Square:
                sinButton.setEnabled(true);
                stButton.setEnabled(true);
                sqButton.setEnabled(false);
                break;
                
            default:
                break;
        }
                        
    }
}

void MIDISynthZiMengAudioProcessorEditor::timerCallback(){
    
    if (waveType != audioProcessor.toneBank->getWaveType()){
        
        waveType = audioProcessor.toneBank->getWaveType();
        
        switch (waveType) {
                case Tone::Sine:
                    sinButton.setEnabled(false);
                    stButton.setEnabled(true);
                    sqButton.setEnabled(true);
                    break;
                
                case Tone::Sawtooth:
                    sinButton.setEnabled(true);
                    stButton.setEnabled(false);
                    sqButton.setEnabled(true);
                    break;
                
                case Tone::Square:
                    sinButton.setEnabled(true);
                    stButton.setEnabled(true);
                    sqButton.setEnabled(false);
                    break;
                    
                default:
                    break;
        }
    }
    
}
