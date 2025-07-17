#pragma once

#include <JuceHeader.h>

class StemProgressBar : public juce::Component,
                       public juce::Timer
{
public:
    StemProgressBar();
    ~StemProgressBar() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    
    void setProgress(double progress);
    void setComplete(bool complete);
    void setWaitingState(bool waiting);
    void reset();
    void setStatusText(const juce::String& text);
    
    bool isComplete() const { return isCompleted; }
    
private:
    double currentProgress;
    bool isCompleted;
    bool isWaiting;
    float glowIntensity;
    float breathingPhase;
    juce::String statusText;
    
    void updateGlow();
    void updateBreathing();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StemProgressBar)
};