#include "ProgressBar.h"

StemProgressBar::StemProgressBar()
    : currentProgress(0.0),
      isCompleted(false),
      glowIntensity(0.0f),
      breathingPhase(0.0f)
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
    
    // Make the bar thinner and centered
    auto barHeight = originalBounds.getHeight() * 0.5f;
    auto barBounds = originalBounds.withHeight(barHeight).withY(originalBounds.getCentreY() - barHeight * 0.5f);

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
        glowIntensity = 0.0f;
    }
    repaint();
}

void StemProgressBar::reset()
{
    currentProgress = 0.0;
    isCompleted = false;
    glowIntensity = 0.0f;
    breathingPhase = 0.0f;
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