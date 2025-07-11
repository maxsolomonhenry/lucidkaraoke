#include "SplitButton.h"

SplitButton::SplitButton()
    : isHovered(false), isEnabled(false), isProcessingState(false)
{
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

SplitButton::~SplitButton()
{
}

void SplitButton::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto cornerSize = 8.0f;
    
    // Determine button color based on state
    juce::Colour buttonColor;
    if (!isEnabled)
        buttonColor = juce::Colour(0xff1a1a1a); // Darker when disabled
    else if (isProcessingState)
        buttonColor = juce::Colour(0xff4dabf7).darker(0.3f); // Blue when processing
    else if (isHovered)
        buttonColor = juce::Colour(0xff404040); // Lighter when hovered
    else
        buttonColor = juce::Colour(0xff2d2d2d); // Default dark surface
    
    g.setColour(buttonColor);
    g.fillRoundedRectangle(bounds.toFloat(), cornerSize);
    
    // Border color
    juce::Colour borderColor;
    if (!isEnabled)
        borderColor = juce::Colour(0xff2d2d2d); // Muted border when disabled
    else if (isProcessingState)
        borderColor = juce::Colour(0xff4dabf7); // Blue border when processing
    else
        borderColor = juce::Colour(0xff4dabf7); // Default blue border
    
    g.setColour(borderColor);
    g.drawRoundedRectangle(bounds.toFloat(), cornerSize, 2.0f);
    
    // Draw icon or spinner
    if (isProcessingState)
    {
        drawSpinner(g, bounds);
    }
    else
    {
        drawSplitIcon(g, bounds);
    }
    
    // Draw text
    g.setColour(isEnabled ? juce::Colour(0xffe9ecef) : juce::Colour(0xff868e96));
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    
    auto textBounds = bounds.reduced(0, bounds.getHeight() / 3);
    juce::String buttonText = isProcessingState ? "PROCESSING..." : "SPLIT STEMS";
    g.drawText(buttonText, textBounds, juce::Justification::centred);
}

void SplitButton::resized()
{
}

void SplitButton::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isLeftButtonDown() && isEnabled && !isProcessingState)
    {
        if (onSplitRequested)
            onSplitRequested(juce::File()); // Pass empty file for now, will be updated later
    }
}

void SplitButton::mouseEnter(const juce::MouseEvent& /*event*/)
{
    if (isEnabled && !isProcessingState)
    {
        isHovered = true;
        repaint();
    }
}

void SplitButton::mouseExit(const juce::MouseEvent& /*event*/)
{
    isHovered = false;
    repaint();
}

void SplitButton::setEnabled(bool enabled)
{
    if (isEnabled != enabled)
    {
        isEnabled = enabled;
        setMouseCursor(enabled ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
        repaint();
    }
}

void SplitButton::setProcessing(bool processing)
{
    if (isProcessingState != processing)
    {
        isProcessingState = processing;
        repaint();
    }
}

bool SplitButton::isProcessing() const
{
    return isProcessingState;
}

void SplitButton::drawSplitIcon(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto iconBounds = bounds.reduced(bounds.getWidth() / 3, bounds.getHeight() / 2);
    iconBounds = iconBounds.withHeight(iconBounds.getHeight() / 2);
    
    juce::Colour iconColor = isEnabled ? juce::Colour(0xff4dabf7) : juce::Colour(0xff868e96);
    g.setColour(iconColor);
    
    // Draw a fork/split icon representing stem separation
    auto centerX = iconBounds.getCentreX();
    auto centerY = iconBounds.getCentreY();
    auto radius = iconBounds.getWidth() / 4;
    
    // Main stem (vertical line)
    g.drawLine(centerX, iconBounds.getY(), centerX, centerY, 2.0f);
    
    // Split branches
    g.drawLine(centerX, centerY, centerX - radius, centerY + radius, 2.0f);
    g.drawLine(centerX, centerY, centerX + radius, centerY + radius, 2.0f);
    
    // Add small circles at the end of branches to represent stems
    g.fillEllipse(centerX - radius - 2, centerY + radius - 2, 4, 4);
    g.fillEllipse(centerX + radius - 2, centerY + radius - 2, 4, 4);
    
    // Add additional smaller branches
    g.drawLine(centerX - radius, centerY + radius, centerX - radius - radius/2, centerY + radius + radius/2, 1.5f);
    g.drawLine(centerX + radius, centerY + radius, centerX + radius + radius/2, centerY + radius + radius/2, 1.5f);
}

void SplitButton::drawSpinner(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto iconBounds = bounds.reduced(bounds.getWidth() / 3, bounds.getHeight() / 2);
    iconBounds = iconBounds.withHeight(iconBounds.getHeight() / 2);
    
    g.setColour(juce::Colour(0xff4dabf7));
    
    // Simple rotating spinner
    auto centerX = iconBounds.getCentreX();
    auto centerY = iconBounds.getCentreY();
    auto radius = iconBounds.getWidth() / 4;
    
    // Get current time for animation
    auto currentTime = juce::Time::getMillisecondCounter();
    auto angle = (currentTime / 10) % 360; // Rotate every 10ms
    
    // Draw spinning arc
    juce::Path spinnerPath;
    spinnerPath.addArc(centerX - radius, centerY - radius, radius * 2, radius * 2, 
                      juce::degreesToRadians(angle), juce::degreesToRadians(angle + 270));
    
    g.strokePath(spinnerPath, juce::PathStrokeType(2.0f));
}