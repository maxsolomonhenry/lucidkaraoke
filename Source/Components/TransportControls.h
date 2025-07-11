#pragma once

#include <JuceHeader.h>
#include "RecordButton.h"

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
    std::function<void(bool)> onRecordStateChanged;
    
    void setPlayButtonEnabled(bool enabled);
    void setPauseButtonEnabled(bool enabled);
    void setStopButtonEnabled(bool enabled);
    
    void setRecordingState(bool recording);
    
private:
    std::unique_ptr<juce::TextButton> playButton;
    std::unique_ptr<juce::TextButton> pauseButton;
    std::unique_ptr<juce::TextButton> stopButton;
    std::unique_ptr<RecordButton> recordButton;
    
    void drawPlayIcon(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawPauseIcon(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawStopIcon(juce::Graphics& g, juce::Rectangle<int> bounds);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportControls)
};