#include "RVCProcessor.h"

RVCProcessor::RVCProcessor(const juce::File& inputVocalFile, const juce::File& outputFile, const juce::String& modelPath)
    : Thread("RVCProcessor"),
      inputVocalFile(inputVocalFile),
      outputFile(outputFile),
      modelPath(modelPath)
{
    updateProgress(0.0, "Initializing RVC processor...");
}

RVCProcessor::~RVCProcessor()
{
}

void RVCProcessor::run()
{
    updateProgress(0.1, "Checking RVC availability...");
    
    if (!checkRVCAvailability())
    {
        if (onProcessingComplete)
            onProcessingComplete(false, "RVC environment is not working properly. This might be due to:\n\n"
                                      "1. Missing RVC dependencies\n"
                                      "2. Python environment issues\n"
                                      "3. Missing pre-trained models\n\n"
                                      "Please check the demucs_env installation.");
        return;
    }
    
    updateProgress(0.2, "Verifying input files...");
    
    if (!inputVocalFile.exists())
    {
        if (onProcessingComplete)
            onProcessingComplete(false, "Input vocal file not found: " + inputVocalFile.getFullPathName());
        return;
    }
    
    if (modelPath.isEmpty())
    {
        if (onProcessingComplete)
            onProcessingComplete(false, "No RVC model specified. Please select an RVC model file (.pth).");
        return;
    }
    
    juce::File modelFile(modelPath);
    if (!modelFile.exists())
    {
        if (onProcessingComplete)
            onProcessingComplete(false, "RVC model file not found: " + modelPath);
        return;
    }
    
    updateProgress(0.3, "Preparing RVC processing...");
    
    // Create output directory if it doesn't exist
    auto outputDir = outputFile.getParentDirectory();
    if (!outputDir.exists())
    {
        if (!outputDir.createDirectory())
        {
            if (onProcessingComplete)
                onProcessingComplete(false, "Failed to create output directory: " + outputDir.getFullPathName());
            return;
        }
    }
    
    updateProgress(0.4, "Building RVC command...");
    
    juce::String command = buildRVCCommand();
    
    if (threadShouldExit())
        return;
    
    updateProgress(0.5, "Processing voice conversion...");
    bool success = executeRVCCommand(command);
    
    if (threadShouldExit())
        return;
    
    if (success)
    {
        updateProgress(1.0, "Voice conversion complete!");
        if (onProcessingComplete)
            onProcessingComplete(true, "Voice conversion has been successfully completed!\n\nOutput: " + outputFile.getFullPathName());
    }
}

bool RVCProcessor::checkRVCAvailability()
{
    juce::ChildProcess process;
    
    // Test the virtual environment's Python with basic imports
    juce::String testCommand = "/Users/maxhenry/Documents/cpp/lucidkaraoke/demucs_env/bin/python3 -c \"import torch; import librosa; import soundfile; print('RVC dependencies available')\"";
    
    if (process.start(testCommand))
    {
        process.waitForProcessToFinish(10000);
        return process.getExitCode() == 0;
    }
    
    return false;
}

juce::String RVCProcessor::buildRVCCommand()
{
    juce::String pythonExecutable = "/Users/maxhenry/Documents/cpp/lucidkaraoke/demucs_env/bin/python3";
    
    // For now, we'll use a simple Python script approach
    // Later we can integrate with a full RVC repository
    juce::String rvcScript = "/Users/maxhenry/Documents/cpp/lucidkaraoke/rvc_simple_inference.py";
    
    juce::StringArray args;
    args.add(pythonExecutable);
    args.add(rvcScript);
    args.add("--input");
    args.add(inputVocalFile.getFullPathName());
    args.add("--output");
    args.add(outputFile.getFullPathName());
    args.add("--model");
    args.add(modelPath);
    args.add("--f0_method");
    args.add(f0Method);
    args.add("--pitch");
    args.add(juce::String(pitchShift));
    args.add("--quality");
    args.add(juce::String(quality));
    
    return args.joinIntoString(" ");
}

bool RVCProcessor::executeRVCCommand(const juce::String& command)
{
    juce::ChildProcess process;
    
    updateProgress(0.6, "Starting RVC inference...");
    
    // Write the command to a debug file
    juce::File debugFile("/tmp/rvc_command.txt");
    debugFile.replaceWithText(command);
    
    if (!process.start(command))
    {
        updateProgress(0.6, "Failed to start RVC process");
        if (onProcessingComplete)
            onProcessingComplete(false, "Failed to start RVC inference process");
        return false;
    }
    
    juce::String processOutput;
    
    // Monitor the process with progress updates
    int timeoutMs = 180000; // 3 minutes timeout for voice conversion
    int elapsedMs = 0;
    int checkIntervalMs = 2000;
    
    while (process.isRunning() && elapsedMs < timeoutMs)
    {
        if (threadShouldExit())
        {
            process.kill();
            return false;
        }
        
        // Try to read any available output
        auto currentOutput = process.readAllProcessOutput();
        if (currentOutput.isNotEmpty())
        {
            processOutput += currentOutput;
            juce::File outputDebugFile("/tmp/rvc_process_output.txt");
            outputDebugFile.replaceWithText(processOutput);
            
            // Look for progress indicators
            if (currentOutput.contains("Processing") || currentOutput.contains("%"))
            {
                updateProgress(0.6 + (0.3 * static_cast<double>(elapsedMs) / timeoutMs), 
                             "RVC: " + currentOutput.substring(0, 80) + "...");
            }
        }
        
        // Update progress based on elapsed time
        double progress = 0.6 + (0.3 * static_cast<double>(elapsedMs) / timeoutMs);
        
        if (elapsedMs % 10000 == 0) // Every 10 seconds
        {
            updateProgress(juce::jmin(progress, 0.9), 
                         "Converting voice... (" + juce::String(elapsedMs / 1000) + "s elapsed)");
        }
        
        juce::Thread::sleep(checkIntervalMs);
        elapsedMs += checkIntervalMs;
    }
    
    if (process.isRunning())
    {
        process.kill();
        updateProgress(0.9, "RVC process timed out");
        if (onProcessingComplete)
            onProcessingComplete(false, "Voice conversion process timed out");
        return false;
    }
    
    int exitCode = process.getExitCode();
    
    // Read any remaining output
    auto remainingOutput = process.readAllProcessOutput();
    if (remainingOutput.isNotEmpty())
    {
        processOutput += remainingOutput;
    }
    
    updateProgress(0.95, "RVC finished with exit code: " + juce::String(exitCode));
    
    if (processOutput.isNotEmpty())
    {
        juce::File outputDebugFile("/tmp/rvc_process_output.txt");
        outputDebugFile.replaceWithText(processOutput);
        
        if (onProcessingComplete)
        {
            if (exitCode == 0)
            {
                // Verify output file was created
                if (outputFile.exists())
                    onProcessingComplete(true, "Voice conversion completed successfully!");
                else
                    onProcessingComplete(false, "RVC process completed but output file was not created");
            }
            else
            {
                juce::String errorMsg = "RVC failed with exit code " + juce::String(exitCode) + ":\n\n";
                errorMsg += processOutput;
                onProcessingComplete(false, errorMsg);
            }
        }
        return exitCode == 0 && outputFile.exists();
    }
    
    return exitCode == 0 && outputFile.exists();
}

void RVCProcessor::setModelPath(const juce::String& modelPath)
{
    this->modelPath = modelPath;
}

void RVCProcessor::setF0Method(const juce::String& method)
{
    this->f0Method = method;
}

void RVCProcessor::setPitchShift(float semitones)
{
    this->pitchShift = semitones;
}

void RVCProcessor::setQuality(int quality)
{
    this->quality = quality;
}

void RVCProcessor::updateProgress(double progress, const juce::String& message)
{
    if (onProgressUpdate)
    {
        juce::MessageManager::callAsync([this, progress, message]() {
            onProgressUpdate(progress, message);
        });
    }
}