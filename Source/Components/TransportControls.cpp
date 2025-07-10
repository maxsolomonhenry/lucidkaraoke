#include "TransportControls.h"

TransportControls::TransportControls()
{
    playButton = std::make_unique<juce::TextButton>("Play");
    pauseButton = std::make_unique<juce::TextButton>("Pause");
    stopButton = std::make_unique<juce::TextButton>("Stop");
    
    playButton->onClick = [this]() { 
        if (onPlayClicked) onPlayClicked(); 
    };
    
    pauseButton->onClick = [this]() { 
        if (onPauseClicked) onPauseClicked(); 
    };
    
    stopButton->onClick = [this]() { 
        if (onStopClicked) onStopClicked(); 
    };
    
    addAndMakeVisible(playButton.get());
    addAndMakeVisible(pauseButton.get());
    addAndMakeVisible(stopButton.get());
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
    auto buttonWidth = 60;
    auto buttonHeight = 40;
    auto spacing = 10;
    
    auto totalWidth = (buttonWidth * 3) + (spacing * 2);
    auto startX = (bounds.getWidth() - totalWidth) / 2;
    auto startY = (bounds.getHeight() - buttonHeight) / 2;
    
    playButton->setBounds(startX, startY, buttonWidth, buttonHeight);
    pauseButton->setBounds(startX + buttonWidth + spacing, startY, buttonWidth, buttonHeight);
    stopButton->setBounds(startX + (buttonWidth + spacing) * 2, startY, buttonWidth, buttonHeight);
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

void TransportControls::drawPlayIcon(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto iconBounds = bounds.reduced(bounds.getWidth() / 3);
    
    g.setColour(juce::Colour(0xffe9ecef));
    
    juce::Path playPath;
    playPath.addTriangle(iconBounds.getX(), iconBounds.getY(),
                        iconBounds.getX(), iconBounds.getBottom(),
                        iconBounds.getRight(), iconBounds.getCentreY());
    
    g.fillPath(playPath);
}

void TransportControls::drawPauseIcon(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto iconBounds = bounds.reduced(bounds.getWidth() / 3);
    auto barWidth = iconBounds.getWidth() / 3;
    
    g.setColour(juce::Colour(0xffe9ecef));
    
    g.fillRect(iconBounds.getX(), iconBounds.getY(), barWidth, iconBounds.getHeight());
    g.fillRect(iconBounds.getRight() - barWidth, iconBounds.getY(), barWidth, iconBounds.getHeight());
}

void TransportControls::drawStopIcon(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto iconBounds = bounds.reduced(bounds.getWidth() / 3);
    
    g.setColour(juce::Colour(0xffe9ecef));
    g.fillRect(iconBounds);
}