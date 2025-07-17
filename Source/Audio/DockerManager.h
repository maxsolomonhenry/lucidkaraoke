#pragma once

#include <JuceHeader.h>

/**
 * Utility class for managing Docker containers for the DeMucs service
 */
class DockerManager
{
public:
    DockerManager();
    ~DockerManager();
    
    // Container management
    bool isDockerAvailable();
    bool isContainerRunning(const juce::String& containerName = "");
    bool startContainer(bool useGPU = false);
    bool stopContainer();
    bool buildContainer();
    
    // Service health checking
    bool isServiceHealthy(const juce::String& serviceUrl = "http://localhost:8000");
    bool waitForServiceReady(const juce::String& serviceUrl = "http://localhost:8000", int timeoutSeconds = 60);
    
    // Configuration
    void setProjectRoot(const juce::File& projectRoot);
    void setUseGPU(bool useGPU);
    
    // Status callbacks
    std::function<void(const juce::String& message)> onStatusUpdate;
    std::function<void(const juce::String& error)> onError;
    
private:
    juce::File projectRoot;
    bool useGPU;
    
    juce::String getDockerComposePath();
    juce::String getContainerProfile();
    bool executeDockerCommand(const juce::String& command, int timeoutMs = 30000);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DockerManager)
};