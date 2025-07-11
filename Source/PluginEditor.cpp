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
    // Recording is now automatic with playback - no manual control needed
    transportControls->onRecordStateChanged = nullptr;
    addAndMakeVisible(transportControls.get());

    progressBar = std::make_unique<StemProgressBar>();
    addAndMakeVisible(progressBar.get());

    startTimer(50);

    setSize (800, 600);
    
    // Listen for recording state changes from the processor
    audioProcessor.addChangeListener(this);
}

LucidkaraokeAudioProcessorEditor::~LucidkaraokeAudioProcessorEditor()
{
    audioProcessor.removeChangeListener(this);
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
    
    // Progress bar below load button, above waveform
    auto progressHeight = 12;
    auto progressBounds = bounds.removeFromTop(progressHeight);
    progressBar->setBounds(progressBounds);
    
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
    
}

void LucidkaraokeAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &audioProcessor)
    {
        // Update the recording button state to reflect the current recording state
        transportControls->setRecordingState(audioProcessor.isRecording());
    }
}

void LucidkaraokeAudioProcessorEditor::loadFile(const juce::File& file)
{
    audioProcessor.loadFile(file);
    waveformDisplay->loadURL(juce::URL(file));
    
    // Magic: automatically start stem processing in background
    progressBar->reset();
    splitAudioStems(file);
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
    
    // Create unique temp directory for this processing session
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                       .getChildFile("lucidkaraoke_stems_" + juce::String(juce::Random::getSystemRandom().nextInt64()));
    
    
    // Create and start the stem processor
    auto* processor = new StemProcessor(inputFile, tempDir);
    
    // Wire up progress updates to the progress bar
    processor->onProgressUpdate = [this](double progress, const juce::String& statusMessage) {
        progressBar->setProgress(progress);
        // Optional: could show status message somewhere in UI
    };
    
    processor->onProcessingComplete = [this, tempDir](bool success, const juce::String& message) {
        juce::MessageManager::callAsync([this, success, message, tempDir]() {
            
            if (success)
            {
                progressBar->setComplete(true);
                // Magic is ready! No blocking dialog needed
                // TODO: This is where the magic happens at the end of the song
            }
            else
            {
                progressBar->reset();
                // Only show error messages, not success messages
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "Stem Separation Failed",
                    message
                );
            }
        });
    };
    
    processor->startThread();
}
