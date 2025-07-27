#include "SourceToggleButton.h"

SourceToggleButton::SourceToggleButton()
    : juce::Button("ReplaceToggle"),
      switchPosition(0.0f),
      targetPosition(0.0f),
      isAnimating(false)
{
    setClickingTogglesState(true);
    addAndMakeVisible(switchThumb);
    switchThumb.setWantsKeyboardFocus(false);
    switchThumb.setInterceptsMouseClicks(false, false);
}

SourceToggleButton::~SourceToggleButton()
{
    stopTimer();
}

void SourceToggleButton::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = getLocalBounds().toFloat();
    auto cornerRadius = bounds.getHeight() * 0.5f;
    
    // Background track
    juce::Colour trackColour;
    if (!isEnabled())
    {
        trackColour = juce::Colour(0xff2a2a2a);
    }
    else if (getToggleState())
    {
        trackColour = juce::Colour(0xff51cf66); // Green when on (replace mode)
    }
    else
    {
        trackColour = juce::Colour(0xff666666); // Gray when off (original mode)  
    }
    
    
    if (shouldDrawButtonAsHighlighted && isEnabled())
        trackColour = trackColour.brighter(0.1f);
    
    g.setColour(trackColour);
    g.fillRoundedRectangle(bounds, cornerRadius);
    
    // Border
    g.setColour(juce::Colour(0xff404040));
    g.drawRoundedRectangle(bounds, cornerRadius, 1.0f);
}

void SourceToggleButton::resized()
{
    updateSwitchPosition();
}

void SourceToggleButton::clicked()
{
    // Toggle the state
    juce::Button::setToggleState(!getToggleState(), juce::dontSendNotification);
    
    // Start animation to new position
    targetPosition = getToggleState() ? 1.0f : 0.0f;
    if (!isAnimating)
    {
        isAnimating = true;
        startTimer(16); // ~60fps
    }
    
    // Notify callback
    if (onToggleStateChanged)
        onToggleStateChanged(getToggleState());
}

void SourceToggleButton::setToggleState(bool showingMixed)
{
    juce::Button::setToggleState(showingMixed, juce::dontSendNotification);
    targetPosition = getToggleState() ? 1.0f : 0.0f;
    switchPosition = targetPosition; // Immediate positioning when set programmatically
    updateSwitchPosition();
}

void SourceToggleButton::setEnabled(bool enabled)
{
    juce::Button::setEnabled(enabled);
    switchThumb.setEnabled(enabled);
    repaint();
}

void SourceToggleButton::timerCallback()
{
    // Smooth animation towards target position
    float speed = 0.15f;
    float diff = targetPosition - switchPosition;
    
    if (std::abs(diff) < 0.01f)
    {
        switchPosition = targetPosition;
        isAnimating = false;
        stopTimer();
    }
    else
    {
        switchPosition += diff * speed;
    }
    
    updateSwitchPosition();
}

void SourceToggleButton::updateSwitchPosition()
{
    auto bounds = getLocalBounds();
    auto thumbSize = bounds.getHeight() - 4; // Leave 2px margin on each side
    auto trackWidth = bounds.getWidth() - thumbSize - 4; // Available travel distance
    
    auto thumbX = 2 + (int)(trackWidth * switchPosition);
    auto thumbY = 2;
    
    switchThumb.setBounds(thumbX, thumbY, thumbSize, thumbSize);
}

void SourceToggleButton::SwitchThumb::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Drop shadow
    g.setColour(juce::Colours::black.withAlpha(0.2f));
    g.fillEllipse(bounds.translated(1.0f, 1.0f));
    
    // Main thumb
    juce::ColourGradient gradient(
        juce::Colours::white, bounds.getCentreX(), bounds.getY(),
        juce::Colour(0xfff0f0f0), bounds.getCentreX(), bounds.getBottom(),
        false
    );
    
    g.setGradientFill(gradient);
    g.fillEllipse(bounds);
    
    // Border
    g.setColour(juce::Colour(0xffcccccc));
    g.drawEllipse(bounds, 1.0f);
}