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
    // Draw the actual switch in the center-top of the component
    auto componentBounds = getLocalBounds().toFloat();
    auto switchWidth = 50.0f;
    auto switchHeight = 25.0f;
    auto bounds = juce::Rectangle<float>(
        (componentBounds.getWidth() - switchWidth) / 2,  // Center horizontally
        0,  // Top of component
        switchWidth,
        switchHeight
    );
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
    
    // Draw "REPLACE" text below the switch
    auto textY = bounds.getBottom() + 5;
    auto textBounds = juce::Rectangle<float>(0, textY, componentBounds.getWidth(), 20);
    
    // Text color depends on enabled state
    juce::Colour textColour = isEnabled() ? 
        juce::Colours::white.withAlpha(0.8f) :  // Normal state
        juce::Colours::white.withAlpha(0.3f);   // Disabled state (grayed out)
    
    g.setColour(textColour);
    g.setFont(juce::FontOptions(10.0f, juce::Font::plain));
    g.drawText("REPLACE", textBounds, juce::Justification::centred);
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
    // Use the same switch bounds as in paintButton
    auto componentBounds = getLocalBounds();
    auto switchWidth = 50;
    auto switchHeight = 25;
    auto switchX = (componentBounds.getWidth() - switchWidth) / 2;
    auto switchY = 0;
    
    auto thumbSize = switchHeight - 4; // Leave 2px margin on each side
    auto trackWidth = switchWidth - thumbSize - 4; // Available travel distance
    
    auto thumbX = switchX + 2 + (int)(trackWidth * switchPosition);
    auto thumbY = switchY + 2;
    
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