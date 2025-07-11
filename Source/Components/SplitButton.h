#pragma once

#include <JuceHeader.h>

class SplitButton : public juce::Component
{
public:
    SplitButton();
    ~SplitButton() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    
    std::function<void(const juce::File&)> onSplitRequested;
    
    void setEnabled(bool enabled);
    void setProcessing(bool processing);
    bool isProcessing() const;
    
private:
    bool isHovered;
    bool isEnabled;
    bool isProcessingState;
    
    void drawSplitIcon(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawSpinner(juce::Graphics& g, juce::Rectangle<int> bounds);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SplitButton)
};