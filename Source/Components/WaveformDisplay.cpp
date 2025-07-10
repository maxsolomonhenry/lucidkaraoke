#include "WaveformDisplay.h"
#include <cmath>

WaveformDisplay::WaveformDisplay()
    : thumbnailCache(10),
      audioThumbnail(1000, formatManager, thumbnailCache),
      fileLoaded(false),
      position(0.0)
{
    formatManager.registerBasicFormats();
    audioThumbnail.addChangeListener(this);
    startTimer(40);
}

WaveformDisplay::~WaveformDisplay()
{
    audioThumbnail.removeChangeListener(this);
    stopTimer();
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRect(bounds);
    
    g.setColour(juce::Colour(0xff2d2d2d));
    g.drawRect(bounds, 2);
    
    if (fileLoaded)
    {
        paintIfFileLoaded(g);
        paintPlayhead(g);
    }
    else
    {
        paintIfNoFileLoaded(g);
    }
}

void WaveformDisplay::resized()
{
}

void WaveformDisplay::mouseDown(const juce::MouseEvent& event)
{
    if (fileLoaded && audioThumbnail.getTotalLength() > 0.0)
    {
        auto clickPosition = event.position.x / getWidth();
        position = clickPosition;
        
        if (onPositionChanged)
            onPositionChanged(position);
            
        repaint();
    }
}

void WaveformDisplay::loadURL(const juce::URL& url)
{
    audioThumbnail.clear();
    
    auto* reader = formatManager.createReaderFor(url.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)));
    
    if (reader != nullptr)
    {
        audioThumbnail.setSource(new juce::FileInputSource(url.getLocalFile()));
        fileLoaded = true;
        position = 0.0;
    }
    else
    {
        fileLoaded = false;
    }
    
    repaint();
}

void WaveformDisplay::setPositionRelative(double newPosition)
{
    if (std::abs(position - newPosition) > 0.001)
    {
        position = newPosition;
        repaint();
    }
}

void WaveformDisplay::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &audioThumbnail)
        repaint();
}

void WaveformDisplay::timerCallback()
{
    repaint();
}

void WaveformDisplay::paintIfNoFileLoaded(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff868e96));
    g.setFont(juce::FontOptions(16.0f));
    g.drawText("No audio file loaded", getLocalBounds(), juce::Justification::centred);
}

void WaveformDisplay::paintIfFileLoaded(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto waveformBounds = bounds.reduced(4);
    
    g.setColour(juce::Colour(0xff4dabf7).withAlpha(0.8f));
    audioThumbnail.drawChannels(g, waveformBounds, 0.0, audioThumbnail.getTotalLength(), 1.0f);
    
    g.setColour(juce::Colour(0xff4dabf7).withAlpha(0.3f));
    g.fillRect(waveformBounds.getX(), waveformBounds.getY(), 
               static_cast<int>(waveformBounds.getWidth() * position), waveformBounds.getHeight());
}

void WaveformDisplay::paintPlayhead(juce::Graphics& g)
{
    if (fileLoaded)
    {
        auto bounds = getLocalBounds().reduced(4);
        auto playheadX = bounds.getX() + static_cast<int>(bounds.getWidth() * position);
        
        g.setColour(juce::Colour(0xffe9ecef));
        g.drawLine(playheadX, bounds.getY(), playheadX, bounds.getBottom(), 2.0f);
        
        g.setColour(juce::Colour(0xff4dabf7));
        g.fillEllipse(playheadX - 4, bounds.getY() - 4, 8, 8);
    }
}