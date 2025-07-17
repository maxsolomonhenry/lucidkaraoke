#include "DockerManager.h"

DockerManager::DockerManager()
    : useGPU(false)
{
    // Try to detect project root
    juce::File currentDir = juce::File::getCurrentWorkingDirectory();
    juce::File candidateRoot = currentDir;
    
    // Look for project markers
    while (candidateRoot != juce::File())
    {
        if (candidateRoot.getChildFile("CMakeLists.txt").exists() ||
            candidateRoot.getChildFile("CLAUDE.md").exists() ||
            candidateRoot.getChildFile("docker").exists())
        {
            projectRoot = candidateRoot;
            break;
        }
        candidateRoot = candidateRoot.getParentDirectory();
    }
    
    if (projectRoot == juce::File())
    {
        projectRoot = currentDir; // Fallback to current directory
    }
}

DockerManager::~DockerManager()
{
}

void DockerManager::setProjectRoot(const juce::File& root)
{
    projectRoot = root;
}

void DockerManager::setUseGPU(bool gpu)
{
    useGPU = gpu;
}

bool DockerManager::isDockerAvailable()
{
    juce::ChildProcess checkProcess;
    if (!checkProcess.start("docker --version"))
        return false;
    
    return checkProcess.waitForProcessToFinish(5000);
}

bool DockerManager::isContainerRunning(const juce::String& containerName)
{
    juce::ChildProcess checkProcess;
    juce::String command;
    
    if (containerName.isNotEmpty())
    {
        command = "docker ps --filter \"name=" + containerName + "\" --format \"{{.Names}}\"";
    }
    else
    {
        // Check for any DeMucs container
        command = "docker ps --filter \"ancestor=lucidkaraoke-demucs\" --format \"{{.Names}}\"";
    }
    
    if (!checkProcess.start(command))
        return false;
    
    bool result = checkProcess.waitForProcessToFinish(5000);
    if (!result)
        return false;
    
    juce::String output = checkProcess.readAllProcessOutput();
    return output.trim().isNotEmpty();
}

bool DockerManager::startContainer(bool gpu)
{
    if (!isDockerAvailable())
    {
        if (onError)
            onError("Docker is not available. Please install Docker Desktop.");
        return false;
    }
    
    juce::String dockerComposePath = getDockerComposePath();
    if (dockerComposePath.isEmpty())
    {
        if (onError)
            onError("Could not find docker-compose.yml file.");
        return false;
    }
    
    if (onStatusUpdate)
        onStatusUpdate("Starting Docker container...");
    
    useGPU = gpu;
    juce::String profile = getContainerProfile();
    juce::String command = "docker compose -f \"" + dockerComposePath + "\" --profile " + profile + " up -d";
    
    return executeDockerCommand(command, 60000); // 60 second timeout
}

bool DockerManager::stopContainer()
{
    juce::String dockerComposePath = getDockerComposePath();
    if (dockerComposePath.isEmpty())
        return false;
    
    if (onStatusUpdate)
        onStatusUpdate("Stopping Docker container...");
    
    juce::String command = "docker compose -f \"" + dockerComposePath + "\" down";
    return executeDockerCommand(command, 30000);
}

bool DockerManager::buildContainer()
{
    juce::String dockerComposePath = getDockerComposePath();
    if (dockerComposePath.isEmpty())
        return false;
    
    if (onStatusUpdate)
        onStatusUpdate("Building Docker container (this may take several minutes)...");
    
    juce::String command = "docker compose -f \"" + dockerComposePath + "\" build";
    return executeDockerCommand(command, 600000); // 10 minute timeout for build
}

bool DockerManager::isServiceHealthy(const juce::String& serviceUrl)
{
    juce::URL healthUrl(serviceUrl + "/health");
    
    juce::URL::DownloadTaskOptions options;
    options.withExtraHeaders("Content-Type: application/json");
    
    std::unique_ptr<juce::URL::DownloadTask> task = healthUrl.downloadToMemory(options, false);
    
    if (!task)
        return false;
    
    // Wait for response with timeout
    int timeout = 5000; // 5 seconds
    int elapsed = 0;
    int checkInterval = 100;
    
    while (!task->isFinished() && elapsed < timeout)
    {
        juce::Thread::sleep(checkInterval);
        elapsed += checkInterval;
    }
    
    if (!task->isFinished())
        return false;
    
    auto result = task->getResult();
    if (result.wasSuccessful() && result.getDataSize() > 0)
    {
        juce::String response = result.getData()->toString();
        return response.contains("healthy") || response.contains("status");
    }
    
    return false;
}

bool DockerManager::waitForServiceReady(const juce::String& serviceUrl, int timeoutSeconds)
{
    int elapsed = 0;
    int checkInterval = 2; // Check every 2 seconds
    
    while (elapsed < timeoutSeconds)
    {
        if (isServiceHealthy(serviceUrl))
        {
            if (onStatusUpdate)
                onStatusUpdate("Service is ready!");
            return true;
        }
        
        juce::Thread::sleep(checkInterval * 1000);
        elapsed += checkInterval;
        
        if (onStatusUpdate)
        {
            onStatusUpdate("Waiting for service to be ready... (" + 
                          juce::String(elapsed) + "/" + juce::String(timeoutSeconds) + "s)");
        }
    }
    
    if (onError)
        onError("Service did not become ready within " + juce::String(timeoutSeconds) + " seconds.");
    
    return false;
}

juce::String DockerManager::getDockerComposePath()
{
    juce::File dockerDir = projectRoot.getChildFile("docker");
    juce::File composeFile = dockerDir.getChildFile("docker-compose.yml");
    
    if (composeFile.exists())
        return composeFile.getFullPathName();
    
    return {};
}

juce::String DockerManager::getContainerProfile()
{
    return useGPU ? "gpu" : "cpu";
}

bool DockerManager::executeDockerCommand(const juce::String& command, int timeoutMs)
{
    juce::ChildProcess process;
    
    if (!process.start(command))
    {
        if (onError)
            onError("Failed to execute Docker command: " + command);
        return false;
    }
    
    bool result = process.waitForProcessToFinish(timeoutMs);
    
    if (!result)
    {
        process.kill();
        if (onError)
            onError("Docker command timed out: " + command);
        return false;
    }
    
    int exitCode = process.getExitCode();
    if (exitCode != 0)
    {
        juce::String errorOutput = process.readAllProcessOutput();
        if (onError)
            onError("Docker command failed (exit code " + juce::String(exitCode) + "): " + errorOutput);
        return false;
    }
    
    return true;
}