#pragma once

#include <JuceHeader.h>

class RVCProcessor : public juce::Thread
{
public:
    RVCProcessor(const juce::File& inputVocalFile, const juce::File& outputFile, const juce::String& modelPath = "");
    ~RVCProcessor() override;
    
    void run() override;
    
    std::function<void(bool success, const juce::String& message)> onProcessingComplete;
    std::function<void(double progress, const juce::String& statusMessage)> onProgressUpdate;
    
    void setModelPath(const juce::String& modelPath);
    void setF0Method(const juce::String& method);
    void setPitchShift(float semitones);
    void setQuality(int quality);
    
private:
    juce::File inputVocalFile;
    juce::File outputFile;
    juce::String modelPath;
    juce::String f0Method = "crepe";
    float pitchShift = 0.0f;
    int quality = 128;
    
    bool checkRVCAvailability();
    juce::String buildRVCCommand();
    bool executeRVCCommand(const juce::String& command);
    
    void updateProgress(double progress, const juce::String& message);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RVCProcessor)
};