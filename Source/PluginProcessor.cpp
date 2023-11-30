/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
MIDISynthZiMengAudioProcessor::MIDISynthZiMengAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), vts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
    toneBank.reset(new ToneBank());
    
//    vts.addParameterListener("waveType", this);
    
//    int wt = (int)*vts.getRawParameterValue("waveType");
////    DBG("vts value is "<< wt);
////    toneBank->setWaveType((Tone::WaveType)wt);
//    switch (wt) {
//        case 0:
//            toneBank->setWaveType(Tone::Sine);
//            break;
//        case 1:
//            toneBank->setWaveType(Tone::Sawtooth);
//            break;
//        case 2:
//            toneBank->setWaveType(Tone::Square);
//            break;
//            
//        default:
//            break;
//    }
}

MIDISynthZiMengAudioProcessor::~MIDISynthZiMengAudioProcessor()
{
}

//==============================================================================
const juce::String MIDISynthZiMengAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MIDISynthZiMengAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MIDISynthZiMengAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MIDISynthZiMengAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MIDISynthZiMengAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MIDISynthZiMengAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MIDISynthZiMengAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MIDISynthZiMengAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MIDISynthZiMengAudioProcessor::getProgramName (int index)
{
    return {};
}

void MIDISynthZiMengAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MIDISynthZiMengAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    
    toneBank->prepareToPlay(sampleRate);
//    toneBank->setWaveType(Tone::Square);
    
    midiMessageCollector.reset (sampleRate);
    
//    vts.addParameterListener("waveType", *this);
//    vts.getParameterAdapter(StringRef);

//    toneBank->setWaveType(*vts.getParameter("waveType"));
}

void MIDISynthZiMengAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MIDISynthZiMengAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
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

void MIDISynthZiMengAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    midiMessageCollector.removeNextBlockOfMessages (midiMessages, buffer.getNumSamples());

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    for(const auto metaData: midiMessages){
        auto message = metaData.getMessage();
        
        if (message.isNoteOn()){
            
            
            switch (message.getNoteNumber()) {
                case 12:
                    toneBank->setWaveType(Tone::Sine);
                    break;
                case 14:
                    toneBank->setWaveType(Tone::Sawtooth);
                    break;
                case 16:
                    toneBank->setWaveType(Tone::Square);
                    break;
                    
                default:
                    double newFrequency = (double) message.getMidiNoteInHertz(message.getNoteNumber());
                    float newVel = message.getFloatVelocity();
                    toneBank->noteOn(newFrequency, newVel, toneBank->getWaveType());
                    break;
                    
            }
        } else if (message.isNoteOff()){
            
            double frequency = (double) message.getMidiNoteInHertz(message.getNoteNumber());
            toneBank->noteOff(frequency);
            
        }
    }
    
//    if(testing){
//        double newFrequency1 = 440;
//        double newFrequency2 = 659.26;
//        float newVel = 0.5f;
//        toneBank->setWaveType(Tone::Sine);
//        
//        toneBank->noteOn(newFrequency1, newVel, toneBank->getWaveType());
//        
//        toneBank->noteOn(newFrequency2, newVel, toneBank->getWaveType());
//    }else{
//        
//        double newFrequency3 = 659.26;
//        
//        toneBank->noteOff(newFrequency3);
//    }
        
//    buffer.clear();
    toneBank->renderBuffer(buffer);

    scopeDataCollector.process (buffer.getReadPointer(0), (size_t) buffer.getNumSamples());
    
    
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
//    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
//    {
//        auto* channelData = buffer.getWritePointer (channel);
//        
//        for (int sampleIndex = 0; sampleIndex < buffer.getNumSamples(); sampleIndex++){
//            buffer.setSample(channel, sampleIndex, .5 * std::sin(juce::MathConstants<float>::twoPi  * counter * 441.0f));
//            counter += 1/getSampleRate();
//        }
//        // ..do something to the data...
//    }
}


//==============================================================================
bool MIDISynthZiMengAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MIDISynthZiMengAudioProcessor::createEditor()
{
    return new MIDISynthZiMengAudioProcessorEditor (*this/*, vts*/);
}

//==============================================================================
void MIDISynthZiMengAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto waveTypeParam = vts.state.getOrCreateChildWithName("waveType", nullptr);
    waveTypeParam.setProperty("waveType", (int)toneBank->getWaveType(), nullptr);
    
//    DBG("saved wavetype: "<< (int)toneBank->getWaveType()<<"\n");
    
    auto state = vts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void MIDISynthZiMengAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(vts.state.getType()))
            vts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
    
    auto waveTypeParam = vts.state.getOrCreateChildWithName("waveType", nullptr);
    int wt = (int)waveTypeParam.getProperty("waveType");
    
//    DBG("read wavetype: "<< wt <<"\n");
    
    switch (wt) {
        case 0:
            toneBank->setWaveType(Tone::Sine);
            break;
        case 1:
            toneBank->setWaveType(Tone::Sawtooth);
            break;
        case 2:
            toneBank->setWaveType(Tone::Square);
            break;

        default:
            break;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MIDISynthZiMengAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout MIDISynthZiMengAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

//    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

//    juce::ParameterID pid1 = juce::ParameterID("sinButton", 1);
//    layout.add(std::make_unique<juce::AudioParameterBool> (pid1, "Sine Button",true));
//    juce::AudioParameter
//
//    juce::ParameterID pid2 = juce::ParameterID("stEnabled", 1);
//    layout.add(std::make_unique<juce::AudioParameterBool> (pid2, "Sawtooth Enabled",false));
//    
//    juce::ParameterID pid3 = juce::ParameterID("sqEnabled", 1);
//    layout.add(std::make_unique<juce::AudioParameterBool> (pid3, "Square Enabled",false));
    
//    juce::ParameterID pid = juce::ParameterID("waveType", 1);
//    layout.add(std::make_unique<juce::AudioParameterInt> (pid, "Wave Type", 0, 2, 1));
    
//    layout.add(std::make_unique<juce::AudioParameterChoice> (pid, "Wave Type", juce::StringArray {"sin", "saw", "square"}, 1));
    
    return layout;

//    return { params.begin(), params.end() };
    
}
