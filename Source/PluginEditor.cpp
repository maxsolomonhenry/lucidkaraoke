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
    
    splitButton = std::make_unique<SplitButton>();
    splitButton->onSplitRequested = [this](const juce::File& /*file*/) {
        auto currentFile = audioProcessor.getLastFileURL().getLocalFile();
        if (currentFile.existsAsFile())
            splitAudioStems(currentFile);
    };
    addAndMakeVisible(splitButton.get());
    
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
    
    bounds.removeFromTop(margin / 2);
    
    auto splitButtonHeight = 60;
    splitButton->setBounds(bounds.removeFromTop(splitButtonHeight));
    
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
    
    bool hasFile = audioProcessor.isLoaded();
    bool isPlaying = audioProcessor.isPlaying();
    bool isPaused = audioProcessor.isPaused();
    
    transportControls->setPlayButtonEnabled(hasFile && !isPlaying);
    transportControls->setPauseButtonEnabled(hasFile && (isPlaying || isPaused));
    transportControls->setStopButtonEnabled(hasFile && (isPlaying || isPaused));
    
    splitButton->setEnabled(hasFile && !splitButton->isProcessing());
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

void LucidkaraokeAudioProcessorEditor::splitAudioStems(const juce::File& inputFile)
{
    if (!inputFile.existsAsFile())
        return;
    
    // Create output directory in the same location as the input file
    auto outputDir = inputFile.getParentDirectory().getChildFile(inputFile.getFileNameWithoutExtension() + "_stems");
    
    splitButton->setProcessing(true);
    
    // Create and start the stem processor
    auto* processor = new StemProcessor(inputFile, outputDir);
    processor->onProcessingComplete = [this, outputDir](bool success, const juce::String& message) {
        juce::MessageManager::callAsync([this, success, message, outputDir]() {
            splitButton->setProcessing(false);
            
            juce::String displayMessage = message;
            if (success)
            {
                displayMessage += "\n\nStems saved to:\n" + outputDir.getFullPathName();
                displayMessage += "\n\nLook for the 'htdemucs_ft' subfolder containing your separated stems.";
            }
            
            juce::AlertWindow::showMessageBoxAsync(
                success ? juce::AlertWindow::InfoIcon : juce::AlertWindow::WarningIcon,
                success ? "Stem Separation Complete" : "Stem Separation Failed",
                displayMessage
            );
        });
    };
    
    processor->launchThread();
}
