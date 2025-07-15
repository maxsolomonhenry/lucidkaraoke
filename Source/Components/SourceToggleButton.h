#pragma once

#include <JuceHeader.h>

class SourceToggleButton : public juce::Component
{
public:
    SourceToggleButton();
    ~SourceToggleButton() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    
    void setToggleState(bool showingMixed);
    bool getToggleState() const { return isShowingMixed; }
    
    void setEnabled(bool enabled);
    bool isEnabled() const { return isButtonEnabled; }
    
    std::function<void(bool)> onToggleStateChanged;
    
private:
    bool isShowingMixed;
    bool isButtonEnabled;
    bool isHovered;
    
    void drawToggleIcon(juce::Graphics& g, juce::Rectangle<float> bounds);
    juce::Colour getButtonColour() const;
    juce::Colour getTextColour() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SourceToggleButton)
};