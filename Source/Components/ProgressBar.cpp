#include "ProgressBar.h"

StemProgressBar::StemProgressBar()
    : currentProgress(0.0),
      isCompleted(false),
      glowIntensity(0.0f)
{
    startTimer(50); // Update animation at 20fps
}

StemProgressBar::~StemProgressBar()
{
    stopTimer();
}

void StemProgressBar::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto height = bounds.getHeight();
    auto width = bounds.getWidth();
    
    // Background
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRoundedRectangle(bounds, height * 0.5f);
    
    // Progress fill
    if (currentProgress > 0.0)
    {
        auto progressWidth = width * currentProgress;
        auto progressBounds = bounds.withWidth(progressWidth);
        
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
        else
        {
            // Blue progress during processing
            g.setColour(juce::Colour(0xff4dabf7));
            g.fillRoundedRectangle(progressBounds, height * 0.5f);
        }
    }
    
    // Subtle border
    g.setColour(juce::Colour(0xff404040));
    g.drawRoundedRectangle(bounds, height * 0.5f, 1.0f);
}

void StemProgressBar::resized()
{
    // Nothing to resize - just a simple bar
}

void StemProgressBar::timerCallback()
{
    if (isCompleted)
    {
        updateGlow();
        repaint();
    }
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
    repaint();
}

void StemProgressBar::updateGlow()
{
    // Gentle breathing glow effect
    static float glowPhase = 0.0f;
    glowPhase += 0.05f;
    glowIntensity = 0.5f + 0.5f * std::sin(glowPhase);
}