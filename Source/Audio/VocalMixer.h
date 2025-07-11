#pragma once

#include <JuceHeader.h>

class VocalMixer : public juce::Thread
{
public:
    VocalMixer(const juce::File& recordingFile, const juce::File& karaokeFile, const juce::File& outputFile);
    ~VocalMixer() override;
    
    void run() override;
    
    std::function<void(bool success, const juce::String& message)> onMixingComplete;
    std::function<void(double progress, const juce::String& statusMessage)> onProgressUpdate;
    
private:
    juce::File recordingFile;
    juce::File karaokeFile;
    juce::File outputFile;
    
    bool checkFFmpegAvailability();
    juce::String buildMixingCommand();
    bool executeMixingCommand(const juce::String& command);
    
    void updateProgress(double progress, const juce::String& message);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalMixer)
};