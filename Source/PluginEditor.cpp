/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Config/config.h"

//==============================================================================
LucidkaraokeAudioProcessorEditor::LucidkaraokeAudioProcessorEditor (LucidkaraokeAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), stemProcessingInProgress(false), currentPlaybackMode(PlaybackMode::Normal), canToggleBetweenSources(false)
{
    setLookAndFeel(&darkTheme);

    loadButton = std::make_unique<LoadButton>();
    loadButton->onFileSelected = [this](const juce::File& file) {
        loadFile(file);
    };
    addAndMakeVisible(loadButton.get());
    loadButton->setEnabled(false); // Disable until service URL is configured


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

    sourceToggleButton = std::make_unique<SourceToggleButton>();
    sourceToggleButton->onToggleStateChanged = [this](bool showMixed) {
        togglePlaybackSource(showMixed);
    };
    addAndMakeVisible(sourceToggleButton.get());

    startTimer(50);

    setSize (800, 600);
    
    // Listen for recording state changes from the processor
    audioProcessor.addChangeListener(this);

#ifdef SERVICE_URL
    // SERVICE_URL is defined in config.h, use it directly
    serviceUrl = SERVICE_URL;
    loadButton->setEnabled(true);
#else
    // SERVICE_URL not defined, prompt user for configuration
    promptForServiceUrl();
#endif
}

void LucidkaraokeAudioProcessorEditor::promptForServiceUrl()
{
    auto* w = new juce::AlertWindow ("Configure Cloud Processing Service",
                         "SERVICE_URL is not defined in your build configuration. Please enter the URL for the cloud processing service.\n\nTo avoid this prompt in the future:\n1. Copy Source/Config/config.h.sample to Source/Config/config.h\n2. Set your SERVICE_URL in config.h\n3. Rebuild the application",
                         juce::AlertWindow::NoIcon);

    w->addTextEditor ("serviceUrl", "http://localhost:8000", "Service URL:");

    w->addButton ("OK", 1, juce::KeyPress (juce::KeyPress::returnKey, 0, 0));
    w->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey, 0, 0));

    w->enterModalState (true, juce::ModalCallbackFunction::create ([this, w] (int modalResult)
    {
        if (modalResult == 1) // OK button clicked
        {
            serviceUrl = w->getTextEditorContents("serviceUrl");
        }
        else
        {
            // User cancelled, fall back to default
            serviceUrl = "http://localhost:8000";
        }

        loadButton->setEnabled(true);
        delete w;
    }));
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
    
    // Progress bar below load button, above waveform (increased height for status text)
    auto progressHeight = 32;
    auto progressBounds = bounds.removeFromTop(progressHeight);
    progressBar->setBounds(progressBounds);
    
    bounds.removeFromTop(margin);
    
    auto transportHeight = 80;
    auto transportBounds = bounds.removeFromBottom(transportHeight);
    
    bounds.removeFromBottom(margin);
    
    // Add space for source toggle button above transport controls
    auto toggleHeight = 50;
    auto toggleBounds = bounds.removeFromBottom(toggleHeight);
    bounds.removeFromBottom(margin / 2);
    
    waveformDisplay->setBounds(bounds);
    
    // Position the source toggle button (center it horizontally)
    auto toggleWidth = 100;
    auto centeredToggleBounds = toggleBounds.withWidth(toggleWidth).withCentre(toggleBounds.getCentre());
    sourceToggleButton->setBounds(centeredToggleBounds);
    
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
    
    // Update source toggle button state
    sourceToggleButton->setEnabled(canToggleBetweenSources);
    
}

void LucidkaraokeAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &audioProcessor)
    {
        // Update the recording button state to reflect the current recording state
        transportControls->setRecordingState(audioProcessor.isRecording());
        
        // Check if we have a complete recording that needs vocal mixing
        if (!audioProcessor.isRecording() && audioProcessor.isCompleteRecording())
        {
            handleCompleteRecording();
        }
    }
}

void LucidkaraokeAudioProcessorEditor::loadFile(const juce::File& file)
{
    audioProcessor.loadFile(file);
    waveformDisplay->loadURL(juce::URL(file));
    waveformDisplay->setDisplayMode(WaveformDisplay::DisplayMode::Normal);
    
    // Reset to normal playback mode
    currentPlaybackMode = PlaybackMode::Normal;
    audioProcessor.setRecordingEnabled(true);
    
    // Track current input file for vocal mixing later
    currentInputFile = file;
    
    // Reset toggle state
    canToggleBetweenSources = false;
    currentMixedFile = juce::File();
    sourceToggleButton->setToggleState(false);
    
    // Magic: automatically start stem processing in background
    progressBar->reset();
    progressBar->setStatusText("Separating audio stems...");
    splitAudioStems(file);
}

void LucidkaraokeAudioProcessorEditor::loadMixedFile(const juce::File& file)
{
    audioProcessor.loadFile(file);
    waveformDisplay->loadURL(juce::URL(file));
    waveformDisplay->setDisplayMode(WaveformDisplay::DisplayMode::MixedFile);
    
    // Set to mixed file playback mode
    currentPlaybackMode = PlaybackMode::MixedFilePlayback;
    audioProcessor.setRecordingEnabled(false);
    
    // Store mixed file and enable toggle
    currentMixedFile = file;
    canToggleBetweenSources = true;
    sourceToggleButton->setToggleState(true);
    
    // Update progress bar
    progressBar->setComplete(true);
    progressBar->setStatusText("Processing complete - Ready to play");
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
    
    // Track stem output directory for vocal mixing later
    currentStemOutputDir = tempDir;
    
    // Set processing state
    stemProcessingInProgress = true;
    
    // Create and start the stem processor
    auto* processor = new HttpStemProcessor(inputFile, tempDir, serviceUrl);
    
    // Wire up progress updates to the progress bar
    processor->onProgressUpdate = [this](double progress, const juce::String& statusMessage) {
        progressBar->setProgress(progress);
        progressBar->setStatusText(statusMessage);
    };
    
    processor->onProcessingComplete = [this, tempDir](bool success, const juce::String& message) {
        juce::MessageManager::callAsync([this, success, message, tempDir]() {
            // Reset processing state
            stemProcessingInProgress = false;
            
            if (success)
            {
                progressBar->setComplete(true);
                progressBar->setStatusText("Processing complete - Ready to play");
                
                // Check if we have a completed recording waiting for the karaoke track
                if (!audioProcessor.isRecording() && audioProcessor.isCompleteRecording())
                {
                    // Recording is complete and karaoke track is now ready - start mixing
                    handleCompleteRecording();
                }
            }
            else
            {
                progressBar->reset();
                progressBar->setStatusText("Stem separation failed");
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

void LucidkaraokeAudioProcessorEditor::handleCompleteRecording()
{
    // Get the recording file from the processor
    juce::File recordingFile = audioProcessor.getLastRecordingFile();
    
    if (!recordingFile.exists())
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            "Recording Complete",
            "Your recording is complete, but the recording file could not be found.\n"
            "Please check that the recording was saved properly."
        );
        return;
    }
    
    // Build the expected karaoke file path
    juce::File karaokeFile = currentStemOutputDir.getChildFile("karaoke.mp3");
    
    if (!karaokeFile.exists())
    {
        // Set progress bar to orange waiting state
        progressBar->setWaitingState(true);
        progressBar->setStatusText("Waiting on stem separation...");
        return;
    }
    
    // Both files exist - start vocal mixing
    mixVocalsWithKaraoke(recordingFile, karaokeFile);
}

void LucidkaraokeAudioProcessorEditor::mixVocalsWithKaraoke(const juce::File& recordingFile, const juce::File& karaokeFile)
{
    // Create output filename
    juce::String timestamp = juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S");
    juce::String outputFileName = currentInputFile.getFileNameWithoutExtension() + 
                                  "_with_vocals_" + timestamp + ".mp3";
    juce::File outputFile = karaokeFile.getParentDirectory().getChildFile(outputFileName);
    
    // Create vocal mixer with buffer size for latency compensation
    auto* mixer = new VocalMixer(recordingFile, karaokeFile, outputFile, audioProcessor.getRecordingBufferSize());
    
    // Wire up progress updates to the progress bar
    mixer->onProgressUpdate = [this](double progress, const juce::String& statusMessage) {
        progressBar->setProgress(progress);
        progressBar->setStatusText(statusMessage);
    };
    
    mixer->onMixingComplete = [this, outputFile](bool success, const juce::String& message) {
        juce::MessageManager::callAsync([this, success, message, outputFile]() {
            if (success)
            {
                // Automatically load the mixed file for playback
                loadMixedFile(outputFile);
            }
            else
            {
                progressBar->reset();
                progressBar->setStatusText("Vocal mixing failed");
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "Vocal Mixing Failed",
                    message
                );
            }
        });
    };
    
    // Set progress bar to orange during mixing (in prep state)
    progressBar->setWaitingState(true);
    progressBar->setStatusText("Mixing vocals with karaoke...");
    mixer->startThread();
}

void LucidkaraokeAudioProcessorEditor::togglePlaybackSource(bool showMixed)
{
    if (!canToggleBetweenSources)
        return;
    
    if (showMixed && currentMixedFile.exists())
    {
        // Switch to mixed file
        audioProcessor.loadFile(currentMixedFile);
        waveformDisplay->loadURL(juce::URL(currentMixedFile));
        waveformDisplay->setDisplayMode(WaveformDisplay::DisplayMode::MixedFile);
        currentPlaybackMode = PlaybackMode::MixedFilePlayback;
        audioProcessor.setRecordingEnabled(false);
        progressBar->setStatusText("Playing mixed file with vocals");
    }
    else if (!showMixed && currentInputFile.exists())
    {
        // Switch to original file
        audioProcessor.loadFile(currentInputFile);
        waveformDisplay->loadURL(juce::URL(currentInputFile));
        waveformDisplay->setDisplayMode(WaveformDisplay::DisplayMode::Normal);
        currentPlaybackMode = PlaybackMode::Normal;
        audioProcessor.setRecordingEnabled(true);
        progressBar->setStatusText("Playing original file");
    }
}
