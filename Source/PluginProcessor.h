/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class LucidkaraokeAudioProcessor  : public juce::AudioProcessor,
                                   public juce::ChangeListener,
                                   public juce::ChangeBroadcaster
{
public:
    //==============================================================================
    LucidkaraokeAudioProcessor();
    ~LucidkaraokeAudioProcessor() override;

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
    
    //==============================================================================
    // Audio file handling
    void loadFile(const juce::File& file);
    void play();
    void pause();
    void stop();
    void setPosition(double position);
    double getPosition() const;
    double getLength() const;
    bool isPlaying() const;
    bool isPaused() const;
    bool isLoaded() const;
    
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    juce::URL getLastFileURL() const { return lastFileURL; }
    
    //==============================================================================
    // Recording functionality
    void startRecording();
    void stopRecording();
    bool isRecording() const;

private:
    class RecordingCallback : public juce::AudioIODeviceCallback
    {
    public:
        RecordingCallback(LucidkaraokeAudioProcessor& processor) : owner(processor) {}
        
        void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                           int numInputChannels,
                                           float* const* outputChannelData,
                                           int numOutputChannels,
                                           int numSamples, const juce::AudioIODeviceCallbackContext& context) override;
        
        void audioDeviceAboutToStart(juce::AudioIODevice* device) override {}
        void audioDeviceStopped() override {}
        
    private:
        LucidkaraokeAudioProcessor& owner;
    };
    
    friend class RecordingCallback;

private:
    //==============================================================================
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::MixerAudioSource mixerSource;
    
    enum TransportState
    {
        Stopped,
        Playing,
        Paused
    };
    
    TransportState state;
    juce::URL lastFileURL;
    
    void changeState(TransportState newState);
    
    //==============================================================================
    // Recording members
    std::unique_ptr<juce::AudioFormatWriter> writer;
    juce::File recordingFile;
    
    // Independent audio device for recording
    juce::AudioDeviceManager recordingDeviceManager;
    std::unique_ptr<juce::AudioIODeviceCallback> recordingCallback;

    // For threaded recording
    juce::TimeSliceThread backgroundThread { "Audio Recorder Thread" };
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter;
    juce::CriticalSection writerLock;
    std::atomic<juce::AudioFormatWriter::ThreadedWriter*> activeWriter { nullptr };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LucidkaraokeAudioProcessor)
};
