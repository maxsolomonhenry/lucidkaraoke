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
#include "Audio/StemProcessor.h"

//==============================================================================
/**
*/
class LucidkaraokeAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                          public juce::Timer
{
public:
    LucidkaraokeAudioProcessorEditor (LucidkaraokeAudioProcessor&);
    ~LucidkaraokeAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LucidkaraokeAudioProcessor& audioProcessor;
    
    DarkTheme darkTheme;
    
    std::unique_ptr<LoadButton> loadButton;
    std::unique_ptr<WaveformDisplay> waveformDisplay;
    std::unique_ptr<TransportControls> transportControls;
    std::unique_ptr<StemProgressBar> progressBar;
    
    void loadFile(const juce::File& file);
    void updateWaveformPosition();
    void splitAudioStems(const juce::File& inputFile);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LucidkaraokeAudioProcessorEditor)
};
