#pragma once

#include <JuceHeader.h>

/**
 * HTTP-based stem processor that communicates with a stem separation service
 * Makes HTTP requests to a service endpoint for audio stem separation
 */
class HttpStemProcessor : public juce::Thread
{
public:
    HttpStemProcessor(const juce::File& inputFile, const juce::File& outputDirectory, const juce::String& serviceUrl,
                      int maxRetries = 3, int baseDelayMs = 2000, int maxDelayMs = 30000);
    ~HttpStemProcessor() override;
    
    void run() override;
    
    // Callbacks - same interface as original StemProcessor
    std::function<void(bool success, const juce::String& message)> onProcessingComplete;
    std::function<void(double progress, const juce::String& statusMessage)> onProgressUpdate;
    
private:
    juce::File inputFile;
    juce::File outputDirectory;
    juce::String serviceUrl;
    
    // Retry configuration
    int maxRetries;
    int baseDelayMs;
    int maxDelayMs;
    
    // HTTP communication
    bool isServiceAvailable();
    bool sendSeparationRequest();
    bool extractStems(const juce::File& zipFile);
    bool downloadAndExtractStems(const juce::MemoryBlock& zipData);
    
    // Retry logic
    bool isServiceAvailableWithRetry();
    bool sendSeparationRequestWithRetry();
    bool isTransientError(int exitCode, const juce::String& output);
    void waitWithBackoff(int attemptNumber);
    
    // Progress and status
    void updateProgress(double progress, const juce::String& message);
    
    // Post-processing (keeping existing functionality)
    bool generateKaraokeTrack();
    bool processVocalWithRVC();
    bool generateRVCKaraokeTrack();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HttpStemProcessor)
};