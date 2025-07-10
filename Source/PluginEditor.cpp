/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LucidkaraokeAudioProcessorEditor::LucidkaraokeAudioProcessorEditor (LucidkaraokeAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&darkTheme);
    
    loadButton = std::make_unique<LoadButton>();
    loadButton->onFileSelected = [this](const juce::File& file) {
        loadFile(file);
    };
    addAndMakeVisible(loadButton.get());
    
    waveformDisplay = std::make_unique<WaveformDisplay>();
    waveformDisplay->onPositionChanged = [this](double position) {
        audioProcessor.setPosition(position);
    };
    addAndMakeVisible(waveformDisplay.get());
    
    transportControls = std::make_unique<TransportControls>();
    transportControls->onPlayClicked = [this]() {
        audioProcessor.play();
    };
    transportControls->onPauseClicked = [this]() {
        audioProcessor.pause();
    };
    transportControls->onStopClicked = [this]() {
        audioProcessor.stop();
    };
    addAndMakeVisible(transportControls.get());
    
    startTimer(50);
    
    setSize (800, 600);
}

LucidkaraokeAudioProcessorEditor::~LucidkaraokeAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

//==============================================================================
void LucidkaraokeAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    g.setColour(juce::Colour(0xff4dabf7));
    g.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    g.drawText("LucidKaraoke", getLocalBounds().removeFromTop(60), juce::Justification::centred);
}

void LucidkaraokeAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    auto headerHeight = 60;
    bounds.removeFromTop(headerHeight);
    
    auto margin = 20;
    bounds.reduce(margin, margin);
    
    auto loadButtonHeight = 60;
    loadButton->setBounds(bounds.removeFromTop(loadButtonHeight));
    
    bounds.removeFromTop(margin);
    
    auto transportHeight = 80;
    auto transportBounds = bounds.removeFromBottom(transportHeight);
    
    bounds.removeFromBottom(margin);
    
    waveformDisplay->setBounds(bounds);
    transportControls->setBounds(transportBounds);
}

void LucidkaraokeAudioProcessorEditor::timerCallback()
{
    updateWaveformPosition();
    
    transportControls->setPlayButtonEnabled(audioProcessor.isLoaded() && !audioProcessor.isPlaying());
    transportControls->setPauseButtonEnabled(audioProcessor.isPlaying());
    transportControls->setStopButtonEnabled(audioProcessor.isLoaded());
}

void LucidkaraokeAudioProcessorEditor::loadFile(const juce::File& file)
{
    audioProcessor.loadFile(file);
    waveformDisplay->loadURL(juce::URL(file));
}

void LucidkaraokeAudioProcessorEditor::updateWaveformPosition()
{
    if (audioProcessor.isLoaded())
    {
        waveformDisplay->setPositionRelative(audioProcessor.getPosition());
    }
}
