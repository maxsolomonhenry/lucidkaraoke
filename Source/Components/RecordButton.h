#pragma once

#include <JuceHeader.h>

class RecordButton : public juce::Component,
                     public juce::Timer
{
public:
    RecordButton();
    ~RecordButton() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    void mouseDown(const juce::MouseEvent& event) override;
    
    void setRecording(bool recording);
    bool isRecording() const { return isCurrentlyRecording; }
    
    std::function<void(bool)> onRecordStateChanged;
    
private:
    bool isCurrentlyRecording;
    float glowIntensity;
    float glowPhase;
    
    void updateGlow();
    void drawRecordIcon(juce::Graphics& g, juce::Rectangle<float> bounds);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordButton)
};