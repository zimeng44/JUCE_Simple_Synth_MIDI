/*
  ==============================================================================

    MIDISynth.cpp
    Created: 17 Nov 2023 2:33:14am
    Author:  Zi Meng

  ==============================================================================
*/


#include "MIDISynth.h"

Tone::Tone(double newFrequency, float newVelocity, WaveType newWaveType, double
           newSampleRate){
    
    setSampleRate(newSampleRate) ;
    setFrequency(newFrequency);
    setVel(newVelocity);
    setWaveType(newWaveType);
    
}

Tone::~Tone(){
    
}

void Tone::setFrequency(double newFreq){
    frequency.setTargetValue(newFreq);
    frequency.reset(sampleRate, smoothingLengthInSeconds);
//    DBG("add frequency "<< frequency.getNextValue()<<"\n");
}

double Tone::getFrequency(){
    return frequency.getNextValue();
}

void Tone::setVel(float newVel){
    velocity = newVel;
}

void Tone::setGain(double newGain){
    gain = newGain;
}


void Tone::setWaveType(WaveType newWaveType){
    waveType = newWaveType;
}

void Tone::setSampleRate(double newSampleRate){
    sampleRate = newSampleRate;
    phaseDelta = 1.0 / sampleRate;
}

void Tone::setReleased(){
    isReleased = true;
}

void Tone::updateTone() {
    if (!isReleased) {
        if (gain < velocity) {
            gain *= ATTACK_FACTOR;
        }
    } else {
        gain *= DECAY_FACTOR;
    }
}

void Tone::processSample(float &sample){
    sample += (generateWaveSample() * gain);
//    sample *= gain;
}

bool Tone::shouldBeRemoved() const{
    if(gain < 0.00005){
        return true;
    }else{
        return false;
    }
}

double Tone::generateWaveSample(){
    
    double sample = 0.0;
    
    switch (waveType) {
        case Sine:
            
            sample = std::sin(juce::MathConstants<double>::twoPi  * phase * frequency.getNextValue());
            phase += phaseDelta;
            break;
        
        case Sawtooth:
            
            sample = 2.0f * (phase * frequency.getNextValue() - floor(0.5 + phase * frequency.getNextValue())); // Generate sawtooth wave
            phase += phaseDelta;
            break;
        
        case Square:
            sample = 0;
            if(std::sin(juce::MathConstants<double>::twoPi  * phase * frequency.getNextValue()) < 0.0f) {
                sample = -1;
            }else if (std::sin(juce::MathConstants<double>::twoPi  * phase * frequency.getNextValue()) > 0.0f){
                sample = 1;
            }
            phase += phaseDelta;
            break;
            
        default:
            break;
    }
            
    return sample;
}




//==================================================================================================





ToneBank::ToneBank(){
    ATTACK_FACTOR = 1.05;
    DECAY_FACTOR = 0.95;
}

ToneBank::~ToneBank(){
    for(auto& tone : tones){
        tone.reset();
    }
}

void ToneBank::prepareToPlay(double newSampleRate){
    
    sampleRate = newSampleRate;
    
    for(auto& tone : tones){
        tone->setSampleRate(newSampleRate);
    }
}

void ToneBank::setWaveType(Tone::WaveType newWaveType){
    waveType = newWaveType;
}

Tone::WaveType ToneBank::getWaveType(){
    return waveType;
}

void ToneBank::noteOn(double frequency, float velocity,
                      Tone::WaveType waveType){
    
    if (tones.size() >= 5) {
        return;
    }
    
    for(auto& tone : tones){
        if (tone->getFrequency() == frequency) {
            return;
        }
    }

//    DBG("add frequency "<< frequency<<"\n");
    
    tones.push_back(std::make_unique<Tone>(frequency, velocity, waveType, sampleRate));

}

void ToneBank::noteOff(double frequency){
    
    for(auto& tone : tones){
        if (tone->getFrequency() == frequency) {
            tone->setReleased();
//            DBG("take off frequency "<< frequency<<"\n");
        }
    }
    
}

void ToneBank::renderBuffer(juce::AudioBuffer<float>& buffer){
    
//    buffer.clear();
    bool rendered = false;
    
    for (int channel = 0; channel < buffer.getNumChannels(); channel++) {
        
        if (channel > 0) {
            buffer.copyFrom(channel, 0, buffer, 0, 0, buffer.getNumSamples());
//            auto* channelData = buffer.getWritePointer(channel);
//            std::copy(buffer.getReadPointer(0), buffer.getReadPointer(0)+buffer.getNumSamples(), channelData);
            rendered = true;
            break;
        }
        
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sampleIndex = 0; sampleIndex < buffer.getNumSamples(); sampleIndex++){
            
            auto it = tones.begin();
//            std::vector<std::vector<int>::iterator> toBeRemoved;
            
            for(auto& tone : tones){
                            
                if (tone->shouldBeRemoved()) {
                    tones.erase(it);
                    break;
                }
                
                it++;

            }
            
            for(auto& tone : tones){
                
                tone->processSample(channelData[sampleIndex]);
                tone->updateTone();
                
            }
            
        }
        
        buffer.applyGain(0.3f);
        auto max = buffer.findMinMax(channel, 0, buffer.getNumSamples()).getEnd();
        
        if (max >= 1.0 ) {
            buffer.applyGain(1/max);
            
//            rendered = true;
        }
        
        
    }
    
//    if (rendered) {
//        saveOutput(buffer);
//    }
    
    
}

//==================================================================================================

