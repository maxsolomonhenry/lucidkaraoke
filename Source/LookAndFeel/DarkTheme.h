#pragma once

#include <JuceHeader.h>

class DarkTheme : public juce::LookAndFeel_V4
{
public:
    DarkTheme();
    
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown);
    
    void drawTextButton(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted,
                       bool shouldDrawButtonAsDown);
    
private:
    juce::Colour darkBackground { 0xff1a1a1a };
    juce::Colour darkSurface { 0xff2d2d2d };
    juce::Colour darkAccent { 0xff404040 };
    juce::Colour primaryAccent { 0xff4dabf7 };
    juce::Colour textColor { 0xffe9ecef };
    juce::Colour mutedText { 0xff868e96 };
};