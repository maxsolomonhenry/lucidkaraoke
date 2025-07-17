/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LookAndFeel/DarkTheme.h"
#include "Components/WaveformDisplay.h"
#include "Components/TransportControls.h"
#include "Components/LoadButton.h"
#include "Components/ProgressBar.h"
#include "Components/SourceToggleButton.h"
#include "Audio/HttpStemProcessor.h"
#include "Audio/VocalMixer.h"

//==============================================================================
/**
*/
class LucidkaraokeAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                          public juce::Timer,
                                          public juce::ChangeListener
{
public:
    LucidkaraokeAudioProcessorEditor (LucidkaraokeAudioProcessor&);
    ~LucidkaraokeAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    enum class PlaybackMode
    {
        Normal,
        MixedFilePlayback
    };
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LucidkaraokeAudioProcessor& audioProcessor;
    
    DarkTheme darkTheme;
    
    std::unique_ptr<LoadButton> loadButton;
    std::unique_ptr<WaveformDisplay> waveformDisplay;
    std::unique_ptr<TransportControls> transportControls;
    std::unique_ptr<StemProgressBar> progressBar;
    std::unique_ptr<SourceToggleButton> sourceToggleButton;
    
    void loadFile(const juce::File& file);
    void loadMixedFile(const juce::File& file);
    void updateWaveformPosition();
    void splitAudioStems(const juce::File& inputFile);
    void handleCompleteRecording();
    void mixVocalsWithKaraoke(const juce::File& recordingFile, const juce::File& karaokeFile);
    void togglePlaybackSource(bool showMixed);
    
    // Track stem processing for vocal mixing
    juce::File currentStemOutputDir;
    juce::File currentInputFile;
    juce::File currentMixedFile;
    bool stemProcessingInProgress;
    PlaybackMode currentPlaybackMode;
    bool canToggleBetweenSources;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LucidkaraokeAudioProcessorEditor)
};
