#pragma once

#include <JuceHeader.h>
#include "RVCProcessor.h"

class StemProcessor : public juce::Thread
{
public:
    StemProcessor(const juce::File& inputFile, const juce::File& outputDirectory);
    ~StemProcessor() override;
    
    void run() override;
    
    std::function<void(bool success, const juce::String& message)> onProcessingComplete;
    std::function<void(double progress, const juce::String& statusMessage)> onProgressUpdate;
    
private:
    juce::File inputFile;
    juce::File outputDirectory;
    
    bool checkDeMucsAvailability();
    juce::String buildDeMucsCommand();
    bool executeDeMucsCommand(const juce::String& command);
    bool processVocalWithRVC();
    bool generateKaraokeTrack();
    bool generateRVCKaraokeTrack();
    
    void updateProgress(double progress, const juce::String& message);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StemProcessor)
};