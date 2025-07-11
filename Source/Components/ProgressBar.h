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
    void reset();
    
    bool isComplete() const { return isCompleted; }
    
private:
    double currentProgress;
    bool isCompleted;
    float glowIntensity;
    float breathingPhase;
    
    void updateGlow();
    void updateBreathing();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StemProgressBar)
};