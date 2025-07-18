#include "ProgressBar.h"

StemProgressBar::StemProgressBar()
    : currentProgress(0.0),
      isCompleted(false),
      isWaiting(false),
      glowIntensity(0.0f),
      breathingPhase(0.0f),
      statusText("")
{
    startTimer(50); // Update animation at 20fps
}

StemProgressBar::~StemProgressBar()
{
    stopTimer();
}

void StemProgressBar::paint(juce::Graphics& g)
{
    auto originalBounds = getLocalBounds().toFloat();
    
    // Reserve space for text at the bottom
    auto textHeight = 16.0f;
    auto progressAreaBounds = originalBounds.withHeight(originalBounds.getHeight() - textHeight);
    
    // Make the bar thinner and centered in the progress area
    auto barHeight = progressAreaBounds.getHeight() * 0.6f;
    auto barBounds = progressAreaBounds.withHeight(barHeight).withY(progressAreaBounds.getCentreY() - barHeight * 0.5f);

    auto height = barBounds.getHeight();
    auto width = barBounds.getWidth();
    
    // Static background
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRoundedRectangle(barBounds, height * 0.5f);
    
    // Progress fill
    auto progressWidth = juce::jmax(2.0f, static_cast<float>(width * currentProgress));
    auto progressBounds = barBounds.withWidth(progressWidth);
    
    if (isCompleted)
    {
        // Green glow effect when complete
        auto glowAlpha = 0.3f + (glowIntensity * 0.7f);
        g.setColour(juce::Colour(0xff4caf50).withAlpha(glowAlpha));
        
        // Outer glow
        auto glowBounds = progressBounds.expanded(2.0f * glowIntensity);
        g.fillRoundedRectangle(glowBounds, height * 0.5f);
        
        // Inner fill
        g.setColour(juce::Colour(0xff4caf50));
        g.fillRoundedRectangle(progressBounds, height * 0.5f);
    }
    else if (isWaiting)
    {
        // Orange pulsating effect when waiting for stems
        float breathingValue = std::sin(breathingPhase); // Varies from -1.0 to 1.0
        auto waitingColour = juce::Colour(0xffff9800); // Orange color
        juce::Colour pulsatingColour;

        if (breathingValue >= 0.0f)
        {
            // Interpolate from base to light
            pulsatingColour = waitingColour.interpolatedWith(waitingColour.brighter(0.4f), breathingValue);
        }
        else
        {
            // Interpolate from base to dark
            pulsatingColour = waitingColour.interpolatedWith(waitingColour.darker(0.4f), -breathingValue);
        }
        
        g.setColour(pulsatingColour);
        g.fillRoundedRectangle(progressBounds, height * 0.5f);
    }
    else if (currentProgress > 0.0)
    {
        // Blue progress with a breathing color (oscillating brightness)
        float breathingValue = std::sin(breathingPhase); // Varies from -1.0 to 1.0
        auto progressColour = juce::Colour(0xff4dabf7);
        juce::Colour breathingColour;

        if (breathingValue >= 0.0f)
        {
            // Interpolate from base to light
            breathingColour = progressColour.interpolatedWith(progressColour.brighter(0.5f), breathingValue);
        }
        else
        {
            // Interpolate from base to dark
            breathingColour = progressColour.interpolatedWith(progressColour.darker(0.5f), -breathingValue);
        }
        
        g.setColour(breathingColour);
        g.fillRoundedRectangle(progressBounds, height * 0.5f);
    }
    
    // Subtle border for the whole bar
    g.setColour(juce::Colour(0xff404040));
    g.drawRoundedRectangle(barBounds, height * 0.5f, 1.0f);
    
    // Draw status text below the bar
    if (statusText.isNotEmpty())
    {
        auto textBounds = originalBounds.withY(barBounds.getBottom() + 4.0f).withHeight(textHeight);
        g.setColour(juce::Colour(0xffaaaaaa));
        g.setFont(juce::FontOptions(12.0f));
        g.drawText(statusText, textBounds, juce::Justification::centred);
    }
}

void StemProgressBar::resized()
{
    // Nothing to resize - just a simple bar
}

void StemProgressBar::timerCallback()
{
    updateBreathing();
    
    if (isCompleted)
    {
        updateGlow();
    }
    
    repaint();
}

void StemProgressBar::setProgress(double progress)
{
    currentProgress = juce::jlimit(0.0, 1.0, progress);
    repaint();
}

void StemProgressBar::setComplete(bool complete)
{
    isCompleted = complete;
    if (complete)
    {
        currentProgress = 1.0;
        isWaiting = false;
        glowIntensity = 0.0f;
    }
    repaint();
}

void StemProgressBar::setWaitingState(bool waiting)
{
    isWaiting = waiting;
    if (waiting)
    {
        isCompleted = false;
    }
    repaint();
}

void StemProgressBar::reset()
{
    currentProgress = 0.0;
    isCompleted = false;
    isWaiting = false;
    glowIntensity = 0.0f;
    breathingPhase = 0.0f;
    statusText = "";
    repaint();
}

void StemProgressBar::setStatusText(const juce::String& text)
{
    statusText = text;
    repaint();
}

void StemProgressBar::updateGlow()
{
    // Gentle breathing glow effect
    static float glowPhase = 0.0f;
    glowPhase += 0.05f;
    glowIntensity = 0.5f + 0.5f * std::sin(glowPhase);
}

void StemProgressBar::updateBreathing()
{
    // Breathing pulse at ~0.5 Hz (2 second cycle)
    // At 20fps (50ms timer), we increment by 2π/(20*2) = π/20 per frame
    breathingPhase += 0.157f; // π/20 ≈ 0.157
    
    // Keep phase in reasonable range
    if (breathingPhase > 2.0f * 3.14159f)
        breathingPhase -= 2.0f * 3.14159f;
}