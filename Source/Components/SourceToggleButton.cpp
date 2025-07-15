#include "SourceToggleButton.h"

SourceToggleButton::SourceToggleButton()
    : isShowingMixed(false),
      isButtonEnabled(false),
      isHovered(false)
{
}

SourceToggleButton::~SourceToggleButton()
{
}

void SourceToggleButton::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Background
    auto bgColour = getButtonColour();
    if (isHovered && isButtonEnabled)
    {
        bgColour = bgColour.brighter(0.1f);
    }
    
    g.setColour(bgColour);
    g.fillRoundedRectangle(bounds, 4.0f);
    
    // Border
    g.setColour(juce::Colour(0xff404040));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
    
    // Draw toggle icon
    drawToggleIcon(g, bounds);
    
    // Draw text
    g.setColour(getTextColour());
    g.setFont(12.0f);
    
    juce::String text = isShowingMixed ? "Mixed" : "Original";
    g.drawText(text, bounds.withHeight(bounds.getHeight() * 0.3f).withY(bounds.getBottom() - bounds.getHeight() * 0.3f), 
               juce::Justification::centred, true);
}

void SourceToggleButton::resized()
{
    // Nothing specific to resize
}

void SourceToggleButton::mouseDown(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    
    if (!isButtonEnabled)
        return;
        
    setToggleState(!isShowingMixed);
    
    if (onToggleStateChanged)
        onToggleStateChanged(isShowingMixed);
}

void SourceToggleButton::mouseEnter(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    
    if (isButtonEnabled)
    {
        isHovered = true;
        repaint();
    }
}

void SourceToggleButton::mouseExit(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    
    isHovered = false;
    repaint();
}

void SourceToggleButton::setToggleState(bool showingMixed)
{
    if (isShowingMixed != showingMixed)
    {
        isShowingMixed = showingMixed;
        repaint();
    }
}

void SourceToggleButton::setEnabled(bool enabled)
{
    if (isButtonEnabled != enabled)
    {
        isButtonEnabled = enabled;
        repaint();
    }
}

void SourceToggleButton::drawToggleIcon(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Icon area (top 70% of button)
    auto iconBounds = bounds.withHeight(bounds.getHeight() * 0.7f);
    auto iconColour = getTextColour();
    
    g.setColour(iconColour);
    
    if (isShowingMixed)
    {
        // Draw mixed icon (two overlapping waveforms)
        auto wave1 = iconBounds.reduced(8.0f);
        auto wave2 = wave1.translated(4.0f, 2.0f);
        
        // First waveform (blue-ish)
        g.setColour(juce::Colour(0xff4dabf7).withAlpha(0.7f));
        g.drawRect(wave1.withHeight(6.0f).withCentre(wave1.getCentre()), 1.0f);
        
        // Second waveform (green-ish, overlapping)
        g.setColour(juce::Colour(0xff51cf66).withAlpha(0.9f));
        g.drawRect(wave2.withHeight(6.0f).withCentre(wave2.getCentre()), 1.0f);
    }
    else
    {
        // Draw original icon (single waveform)
        auto waveform = iconBounds.reduced(8.0f);
        g.setColour(juce::Colour(0xff4dabf7));
        g.drawRect(waveform.withHeight(6.0f).withCentre(waveform.getCentre()), 1.0f);
        
        // Add some waveform details
        auto center = waveform.getCentre();
        g.drawLine(center.x - 6.0f, center.y - 2.0f, center.x - 2.0f, center.y + 2.0f, 1.0f);
        g.drawLine(center.x + 2.0f, center.y + 2.0f, center.x + 6.0f, center.y - 2.0f, 1.0f);
    }
}

juce::Colour SourceToggleButton::getButtonColour() const
{
    if (!isButtonEnabled)
        return juce::Colour(0xff2a2a2a);
    
    if (isShowingMixed)
        return juce::Colour(0xff51cf66).withAlpha(0.3f); // Green tint for mixed
    else
        return juce::Colour(0xff4dabf7).withAlpha(0.3f); // Blue tint for original
}

juce::Colour SourceToggleButton::getTextColour() const
{
    if (!isButtonEnabled)
        return juce::Colour(0xff666666);
    
    return juce::Colours::white;
}