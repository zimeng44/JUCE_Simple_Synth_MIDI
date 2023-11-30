/*
  ==============================================================================

    MIDISynth.h
    Created: 17 Nov 2023 2:33:31am
    Author:  Zi Meng

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class Tone {
public:
    enum WaveType { Sine, Sawtooth, Square};
    
    Tone(double frequency, float velocity, WaveType waveType, double
       sampleRate);
    ~Tone();
    
    void setSampleRate(double newSampleRate);
    void setWaveType(WaveType newWaveType);
    void setFrequency(double newFrequency);
    void setVel(float newVel);
    double getFrequency();
    void setGain(double newGain);
    void setReleased();
    void updateTone();
    void processSample(float& sample);
    bool shouldBeRemoved() const;

private:
    WaveType waveType;
    juce::SmoothedValue<double> frequency;
    double sampleRate;
    bool isReleased = false; // note held by default
    double gain = 0.0001, ATTACK_FACTOR = 1.05, DECAY_FACTOR = 0.995; // gain is current velocity
    double phase = 0.0, phaseDelta = 0.0; // internal state counter
    double generateWaveSample();
    float velocity;
    static constexpr auto smoothingLengthInSeconds = 0.01;
    
};

//==============================================================================

class ToneBank {
public:
    ToneBank();
    ~ToneBank();
    
    void prepareToPlay(double newSampleRate);
    void setWaveType(Tone::WaveType waveType);
    Tone::WaveType getWaveType();
    void noteOn(double frequency, float velocity,
       Tone::WaveType waveType);
    void noteOff(double frequency);
    void renderBuffer(juce::AudioBuffer<float>& buffer);
    void saveOutput(juce::AudioBuffer<float>& buffer)
    {
        juce::File outputFile(juce::String("/Users/zimeng/analysis" + juce::String(fileVar) + ".wav"));
        fileVar += 1;
        if(outputFile.exists()) outputFile.deleteFile();
        outputFile.create();
        auto fileStream = std::make_unique<juce::FileOutputStream>(outputFile);
        
        if(fileStream == nullptr) return;
        // in real code you must check whether the stream is null before continuing
        auto wavFormat = std::make_unique<juce::WavAudioFormat>();

        std::unique_ptr<juce::AudioFormatWriter> writer (wavFormat->createWriterFor(fileStream.get(), sampleRate, buffer.getNumChannels(), 16, {}, 0));
        if(writer == nullptr) return;
        // again, you need add a check for a nullptr here
        writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
        fileStream->flush();
        writer.reset();
        writer.release();
        fileStream.release();
    }
    
private:
    std::vector<std::unique_ptr<Tone>> tones;
    Tone::WaveType waveType;
    double sampleRate;
    double ATTACK_FACTOR, DECAY_FACTOR;
    int fileVar=0;
};

//==============================================================================
template <typename SampleType>
class AudioBufferQueue
{
public:
    //==============================================================================
    static constexpr size_t order = 9;
    static constexpr size_t bufferSize = 1U << order;
    static constexpr size_t numBuffers = 5;

    //==============================================================================
    void push (const SampleType* dataToPush, size_t numSamples)
    {
        jassert (numSamples <= bufferSize);

        int start1, size1, start2, size2;
        abstractFifo.prepareToWrite (1, start1, size1, start2, size2);

        jassert (size1 <= 1);
        jassert (size2 == 0);

        if (size1 > 0)
            juce::FloatVectorOperations::copy (buffers[(size_t) start1].data(), dataToPush, (int) juce::jmin (bufferSize, numSamples));

        abstractFifo.finishedWrite (size1);
    }

    //==============================================================================
    void pop (SampleType* outputBuffer)
    {
        int start1, size1, start2, size2;
        abstractFifo.prepareToRead (1, start1, size1, start2, size2);

        jassert (size1 <= 1);
        jassert (size2 == 0);

        if (size1 > 0)
            juce::FloatVectorOperations::copy (outputBuffer, buffers[(size_t) start1].data(), (int) bufferSize);

        abstractFifo.finishedRead (size1);
    }

private:
    //==============================================================================
    juce::AbstractFifo abstractFifo { numBuffers };
    std::array<std::array<SampleType, bufferSize>, numBuffers> buffers;
};



//==============================================================================



template <typename SampleType>
class ScopeDataCollector
{
public:
    //==============================================================================
    ScopeDataCollector (AudioBufferQueue<SampleType>& queueToUse)
        : audioBufferQueue (queueToUse)
    {}

    //==============================================================================
    void process (const SampleType* data, size_t numSamples)
    {
        size_t index = 0;

        if (state == State::waitingForTrigger)
        {
            while (index++ < numSamples)
            {
                auto currentSample = *data++;

                if (currentSample >= triggerLevel && prevSample < triggerLevel)
                {
                    numCollected = 0;
                    state = State::collecting;
                    break;
                }

                prevSample = currentSample;
            }
        }

        if (state == State::collecting)
        {
            while (index++ < numSamples)
            {
                buffer[numCollected++] = *data++;

                if (numCollected == buffer.size())
                {
                    audioBufferQueue.push (buffer.data(), buffer.size());
                    state = State::waitingForTrigger;
                    prevSample = SampleType (100);
                    break;
                }
            }
        }
    }

private:
    //==============================================================================
    AudioBufferQueue<SampleType>& audioBufferQueue;
    std::array<SampleType, AudioBufferQueue<SampleType>::bufferSize> buffer;
    size_t numCollected;
    SampleType prevSample = SampleType (100);

    static constexpr auto triggerLevel = SampleType (0.05);

    enum class State { waitingForTrigger, collecting } state { State::waitingForTrigger };
};




//==============================================================================




template <typename SampleType>
class ScopeComponent  : public juce::Component,
                        private juce::Timer
{
public:
    using Queue = AudioBufferQueue<SampleType>;

    //==============================================================================
    ScopeComponent (Queue& queueToUse)
        : audioBufferQueue (queueToUse)
    {
        sampleData.fill (SampleType (0));
        setFramesPerSecond (30);
    }

    //==============================================================================
    void setFramesPerSecond (int framesPerSecond)
    {
        jassert (framesPerSecond > 0 && framesPerSecond < 1000);
        startTimerHz (framesPerSecond);
    }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::black);
        g.setColour (juce::Colours::white);

        auto area = getLocalBounds();
        auto h = (SampleType) area.getHeight();
        auto w = (SampleType) area.getWidth();

        // Oscilloscope
        auto scopeRect = juce::Rectangle<SampleType> { SampleType (0), SampleType (0), w, h/2 };
        plot (sampleData.data(), sampleData.size(), g, scopeRect, SampleType (1), 0);

        // Spectrum
//        auto spectrumRect = juce::Rectangle<SampleType> { SampleType (0), h / 2, w, h / 2 };
//        plot (spectrumData.data(), spectrumData.size() / 4, g, spectrumRect);
    }

    //==============================================================================
    void resized() override {}

private:
    //==============================================================================
    Queue& audioBufferQueue;
    std::array<SampleType, Queue::bufferSize> sampleData;

//    juce::dsp::FFT fft { Queue::order };
//    using WindowFun = juce::dsp::WindowingFunction<SampleType>;
//    WindowFun windowFun { (size_t) fft.getSize(), WindowFun::hann };
//    std::array<SampleType, 2 * Queue::bufferSize> spectrumData;

    //==============================================================================
    void timerCallback() override
    {
        audioBufferQueue.pop (sampleData.data());
//        juce::FloatVectorOperations::copy (spectrumData.data(), sampleData.data(), (int) sampleData.size());

//        auto fftSize = (size_t) fft.getSize();

//        jassert (spectrumData.size() == 2 * fftSize);
//        windowFun.multiplyWithWindowingTable (spectrumData.data(), fftSize);
//        fft.performFrequencyOnlyForwardTransform (spectrumData.data());

//        static constexpr auto mindB = SampleType (-160);
//        static constexpr auto maxdB = SampleType (0);

//        for (auto& s : spectrumData)
//            s = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (s) - juce::Decibels::gainToDecibels (SampleType (fftSize))), mindB, maxdB, SampleType (0), SampleType (1));

        repaint();
    }

    //==============================================================================
    static void plot (const SampleType* data,
                      size_t numSamples,
                      juce::Graphics& g,
                      juce::Rectangle<SampleType> rect,
                      SampleType scaler = SampleType (1),
                      SampleType offset = SampleType (0))
    {
        auto w = rect.getWidth();
        auto h = rect.getHeight();
        auto right = rect.getRight();

        auto center = rect.getBottom() - offset;
        auto gain = h * scaler;

        for (size_t i = 1; i < numSamples; ++i)
            g.drawLine ({ juce::jmap (SampleType (i - 1), SampleType (0), SampleType (numSamples - 1), SampleType (right - w), SampleType (right)),
                          center - gain * data[i - 1],
                          juce::jmap (SampleType (i), SampleType (0), SampleType (numSamples - 1), SampleType (right - w), SampleType (right)),
                          center - gain * data[i] });
    }
};
