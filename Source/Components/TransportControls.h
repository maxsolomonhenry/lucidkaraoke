#pragma once

#include <JuceHeader.h>

class TransportControls : public juce::Component
{
public:
    TransportControls();
    ~TransportControls() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    std::function<void()> onPlayClicked;
    std::function<void()> onPauseClicked;
    std::function<void()> onStopClicked;
    
    void setPlayButtonEnabled(bool enabled);
    void setPauseButtonEnabled(bool enabled);
    void setStopButtonEnabled(bool enabled);
    
private:
    std::unique_ptr<juce::TextButton> playButton;
    std::unique_ptr<juce::TextButton> pauseButton;
    std::unique_ptr<juce::TextButton> stopButton;
    
    void drawPlayIcon(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawPauseIcon(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawStopIcon(juce::Graphics& g, juce::Rectangle<int> bounds);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportControls)
};