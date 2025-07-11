#pragma once

#include <JuceHeader.h>

class StemProcessor : public juce::ThreadWithProgressWindow
{
public:
    StemProcessor(const juce::File& inputFile, const juce::File& outputDirectory);
    ~StemProcessor() override;
    
    void run() override;
    
    std::function<void(bool success, const juce::String& message)> onProcessingComplete;
    
private:
    juce::File inputFile;
    juce::File outputDirectory;
    
    bool checkDeMucsAvailability();
    juce::String buildDeMucsCommand();
    bool executeDeMucsCommand(const juce::String& command);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StemProcessor)
};