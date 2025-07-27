#include "TransportControls.h"

TransportControls::TransportControls()
{
    playButton = std::make_unique<juce::TextButton>("Play");
    pauseButton = std::make_unique<juce::TextButton>("Pause");
    stopButton = std::make_unique<juce::TextButton>("Stop");
    recordButton = std::make_unique<RecordButton>();
    
    // Load SVG icons
    loadSVGIcons();
    
    playButton->onClick = [this]() { 
        if (onPlayClicked) onPlayClicked(); 
    };
    
    pauseButton->onClick = [this]() { 
        if (onPauseClicked) onPauseClicked(); 
    };
    
    stopButton->onClick = [this]() { 
        if (onStopClicked) onStopClicked(); 
    };
    
    recordButton->onRecordStateChanged = [this](bool recording) {
        if (onRecordStateChanged) onRecordStateChanged(recording);
    };
    
    addAndMakeVisible(playButton.get());
    addAndMakeVisible(pauseButton.get());
    addAndMakeVisible(stopButton.get());
    // Record button functionality preserved but not visible in UI
}

TransportControls::~TransportControls()
{
}

void TransportControls::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRect(bounds);
    
    auto playBounds = playButton->getBounds();
    auto pauseBounds = pauseButton->getBounds();
    auto stopBounds = stopButton->getBounds();
    
    drawPlayIcon(g, playBounds);
    drawPauseIcon(g, pauseBounds);
    drawStopIcon(g, stopBounds);
}

void TransportControls::resized()
{
    auto bounds = getLocalBounds();
    auto buttonSize = 50; // Square buttons
    auto spacing = 15;
    
    // Now we have 3 square buttons (play, pause, stop)
    auto totalWidth = (buttonSize * 3) + (spacing * 2);
    auto startX = (bounds.getWidth() - totalWidth) / 2;
    auto startY = (bounds.getHeight() - buttonSize) / 2;
    
    playButton->setBounds(startX, startY, buttonSize, buttonSize);
    pauseButton->setBounds(startX + buttonSize + spacing, startY, buttonSize, buttonSize);
    stopButton->setBounds(startX + (buttonSize + spacing) * 2, startY, buttonSize, buttonSize);
    // Record button not positioned (not visible)
}

void TransportControls::setPlayButtonEnabled(bool enabled)
{
    playButton->setEnabled(enabled);
}

void TransportControls::setPauseButtonEnabled(bool enabled)
{
    pauseButton->setEnabled(enabled);
}

void TransportControls::setStopButtonEnabled(bool enabled)
{
    stopButton->setEnabled(enabled);
}

void TransportControls::setRecordingState(bool recording)
{
    recordButton->setRecording(recording);
}


void TransportControls::loadSVGIcons()
{
    // Load SVG files from Graphics folder
    auto playFile = juce::File::getCurrentWorkingDirectory()
        .getChildFile("Source").getChildFile("Graphics").getChildFile("play-button.svg");
    auto pauseFile = juce::File::getCurrentWorkingDirectory()
        .getChildFile("Source").getChildFile("Graphics").getChildFile("pause-button.svg");
    auto stopFile = juce::File::getCurrentWorkingDirectory()
        .getChildFile("Source").getChildFile("Graphics").getChildFile("stop-button.svg");
    
    if (playFile.exists())
        playIcon = juce::Drawable::createFromImageFile(playFile);
    if (pauseFile.exists())
        pauseIcon = juce::Drawable::createFromImageFile(pauseFile);
    if (stopFile.exists())
        stopIcon = juce::Drawable::createFromImageFile(stopFile);
}

void TransportControls::drawPlayIcon(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (playIcon)
    {
        auto iconBounds = bounds.reduced(bounds.getWidth() / 4).toFloat();
        playIcon->drawWithin(g, iconBounds, juce::RectanglePlacement::centred, 1.0f);
    }
}

void TransportControls::drawPauseIcon(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (pauseIcon)
    {
        auto iconBounds = bounds.reduced(bounds.getWidth() / 4).toFloat();
        pauseIcon->drawWithin(g, iconBounds, juce::RectanglePlacement::centred, 1.0f);
    }
}

void TransportControls::drawStopIcon(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (stopIcon)
    {
        auto iconBounds = bounds.reduced(bounds.getWidth() / 4).toFloat();
        stopIcon->drawWithin(g, iconBounds, juce::RectanglePlacement::centred, 1.0f);
    }
}