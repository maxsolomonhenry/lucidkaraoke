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
                                   public juce::ChangeListener
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
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LucidkaraokeAudioProcessor)
};
