#include "DarkTheme.h"

DarkTheme::DarkTheme()
{
    setColour(juce::ResizableWindow::backgroundColourId, darkBackground);
    setColour(juce::TextButton::buttonColourId, darkSurface);
    setColour(juce::TextButton::buttonOnColourId, primaryAccent);
    setColour(juce::TextButton::textColourOnId, textColor);
    setColour(juce::TextButton::textColourOffId, textColor);
    setColour(juce::ComboBox::backgroundColourId, darkSurface);
    setColour(juce::ComboBox::textColourId, textColor);
    setColour(juce::ComboBox::outlineColourId, darkAccent);
    setColour(juce::ComboBox::buttonColourId, darkAccent);
    setColour(juce::ComboBox::arrowColourId, textColor);
    setColour(juce::PopupMenu::backgroundColourId, darkSurface);
    setColour(juce::PopupMenu::textColourId, textColor);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, primaryAccent);
    setColour(juce::PopupMenu::highlightedTextColourId, textColor);
}

void DarkTheme::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                   bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto cornerSize = 6.0f;
    
    juce::Colour buttonColor = darkSurface;
    
    if (shouldDrawButtonAsDown)
        buttonColor = primaryAccent.darker(0.2f);
    else if (shouldDrawButtonAsHighlighted)
        buttonColor = darkAccent.brighter(0.1f);
    
    g.setColour(buttonColor);
    g.fillRoundedRectangle(bounds, cornerSize);
    
    g.setColour(darkAccent.brighter(0.2f));
    g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
}

void DarkTheme::drawTextButton(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown)
{
    drawButtonBackground(g, button, juce::Colour(), shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
    
    auto bounds = button.getLocalBounds();
    auto font = getTextButtonFont(button, button.getHeight());
    
    g.setFont(font);
    g.setColour(textColor);
    g.drawText(button.getButtonText(), bounds, juce::Justification::centred, true);
}