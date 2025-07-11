#include "RecordButton.h"

RecordButton::RecordButton()
    : isCurrentlyRecording(false),
      glowIntensity(0.0f),
      glowPhase(0.0f)
{
    startTimer(50); // Update animation at 20fps
}

RecordButton::~RecordButton()
{
    stopTimer();
}

void RecordButton::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Make it a circular button
    auto diameter = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.8f;
    auto circleBounds = juce::Rectangle<float>(diameter, diameter)
                           .withCentre(bounds.getCentre());
    
    if (isCurrentlyRecording)
    {
        // Red glow effect when recording
        auto glowAlpha = 0.3f + (glowIntensity * 0.7f);
        g.setColour(juce::Colour(0xfff44336).withAlpha(glowAlpha));
        
        // Outer glow
        auto glowBounds = circleBounds.expanded(3.0f * glowIntensity);
        g.fillEllipse(glowBounds);
        
        // Inner fill - bright red when recording
        g.setColour(juce::Colour(0xfff44336));
        g.fillEllipse(circleBounds);
    }
    else
    {
        // Default state - dark red/gray
        g.setColour(juce::Colour(0xff666666));
        g.fillEllipse(circleBounds);
    }
    
    // Draw record icon (circle)
    drawRecordIcon(g, circleBounds);
    
    // Border
    g.setColour(juce::Colour(0xff404040));
    g.drawEllipse(circleBounds, 1.0f);
}

void RecordButton::resized()
{
    // Nothing specific to resize
}

void RecordButton::timerCallback()
{
    if (isCurrentlyRecording)
    {
        updateGlow();
        repaint();
    }
}

void RecordButton::mouseDown(const juce::MouseEvent& event)
{
    // Recording is now controlled automatically by playback transport
    // Manual recording control is disabled
    juce::ignoreUnused(event);
}

void RecordButton::setRecording(bool recording)
{
    isCurrentlyRecording = recording;
    
    if (!recording)
    {
        glowIntensity = 0.0f;
        glowPhase = 0.0f;
    }
    
    repaint();
}

void RecordButton::updateGlow()
{
    // Breathing glow effect similar to progress bar
    glowPhase += 0.1f;
    glowIntensity = 0.5f + 0.5f * std::sin(glowPhase);
}

void RecordButton::drawRecordIcon(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Draw a smaller filled circle inside for the record symbol
    auto iconRadius = bounds.getWidth() * 0.25f;
    auto iconBounds = juce::Rectangle<float>(iconRadius * 2, iconRadius * 2)
                         .withCentre(bounds.getCentre());
    
    if (isCurrentlyRecording)
    {
        g.setColour(juce::Colours::white);
    }
    else
    {
        g.setColour(juce::Colour(0xfff44336));
    }
    
    g.fillEllipse(iconBounds);
}