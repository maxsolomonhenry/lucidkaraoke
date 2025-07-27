#pragma once

#include <JuceHeader.h>

class SourceToggleButton : public juce::Button, private juce::Timer
{
public:
    SourceToggleButton();
    ~SourceToggleButton() override;
    
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void resized() override;
    void clicked() override;
    
    void setToggleState(bool showingMixed);
    bool getToggleState() const { return juce::Button::getToggleState(); }
    
    void setEnabled(bool enabled);
    
    std::function<void(bool)> onToggleStateChanged;
    
private:
    void timerCallback() override;
    void updateSwitchPosition();
    
    float switchPosition; // 0.0 = left (original), 1.0 = right (replace)
    float targetPosition;
    bool isAnimating;
    
    class SwitchThumb : public juce::Component
    {
    public:
        void paint(juce::Graphics& g) override;
    };
    
    SwitchThumb switchThumb;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SourceToggleButton)
};