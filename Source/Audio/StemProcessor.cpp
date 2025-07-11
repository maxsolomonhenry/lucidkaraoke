#include "StemProcessor.h"

StemProcessor::StemProcessor(const juce::File& inputFile, const juce::File& outputDirectory)
    : ThreadWithProgressWindow("Processing Audio Stems", true, true),
      inputFile(inputFile),
      outputDirectory(outputDirectory)
{
    setProgress(0.0);
    setStatusMessage("Initializing DeMucs...");
}

StemProcessor::~StemProcessor()
{
}

void StemProcessor::run()
{
    setStatusMessage("Checking DeMucs availability...");
    setProgress(0.1);
    
    if (!checkDeMucsAvailability())
    {
        if (onProcessingComplete)
            onProcessingComplete(false, "DeMucs is not working properly. This might be due to:\n\n"
                                      "1. DeMucs not installed: pip install demucs\n"
                                      "2. FFmpeg not installed: brew install ffmpeg\n"
                                      "3. Missing dependencies: pip install numpy scipy torch\n\n"
                                      "Try running 'demucs --help' in Terminal to test.");
        return;
    }
    
    setStatusMessage("Checking FFmpeg availability...");
    setProgress(0.15);
    
    // Check if FFmpeg is available
    juce::ChildProcess ffmpegCheck;
    if (!ffmpegCheck.start("ffmpeg -version"))
    {
        if (onProcessingComplete)
            onProcessingComplete(false, "FFmpeg is not installed. DeMucs requires FFmpeg to process audio files.\n\n"
                                      "Please install FFmpeg:\n"
                                      "brew install ffmpeg\n\n"
                                      "Then try again.");
        return;
    }
    ffmpegCheck.waitForProcessToFinish(5000);
    
    setStatusMessage("Preparing audio file...");
    setProgress(0.2);
    
    // Create output directory if it doesn't exist
    if (!outputDirectory.exists())
    {
        if (!outputDirectory.createDirectory())
        {
            if (onProcessingComplete)
                onProcessingComplete(false, "Failed to create output directory: " + outputDirectory.getFullPathName());
            return;
        }
    }
    
    setStatusMessage("Running DeMucs stem separation...");
    setProgress(0.3);
    
    juce::String command = buildDeMucsCommand();
    
    if (threadShouldExit())
        return;
    
    setProgress(0.4);
    bool success = executeDeMucsCommand(command);
    
    if (threadShouldExit())
        return;
    
    setProgress(1.0);
    setStatusMessage("Processing complete!");
    
    // Error handling is now done in executeDeMucsCommand
    // Only call completion callback if it wasn't already called
    if (onProcessingComplete && success)
    {
        onProcessingComplete(true, "Stems have been successfully separated!");
    }
}

bool StemProcessor::checkDeMucsAvailability()
{
    juce::ChildProcess process;
    
    // Test the virtual environment's DeMucs directly
    juce::String testCommand = "/Users/maxhenry/Documents/cpp/lucidkaraoke/demucs_env/bin/python3 -m demucs --help";
    
    if (process.start(testCommand))
    {
        process.waitForProcessToFinish(10000); // Wait up to 10 seconds
        return process.getExitCode() == 0;
    }
    
    return false;
}

juce::String StemProcessor::buildDeMucsCommand()
{
    juce::String pythonExecutable = "/Users/maxhenry/Documents/cpp/lucidkaraoke/demucs_env/bin/python3";
    juce::String demucsScript = "/Users/maxhenry/Documents/cpp/lucidkaraoke/demucs_env/bin/demucs";
    
    juce::StringArray args;
    args.add(demucsScript);
    args.add("--mp3");
    args.add("--mp3-bitrate");
    args.add("320");
    args.add("-n");
    args.add("htdemucs_ft");
    args.add("-o");
    args.add(outputDirectory.getFullPathName());
    args.add(inputFile.getFullPathName());
    
    juce::String command;
    command << pythonExecutable << " " << args.joinIntoString(" ");
    
    return command;
}

bool StemProcessor::executeDeMucsCommand(const juce::String& command)
{
    juce::ChildProcess process;
    
    setStatusMessage("Starting DeMucs...");
    
    // Write the command to a debug file so we can see exactly what's being executed
    juce::File debugFile("/tmp/demucs_command.txt");
    debugFile.replaceWithText(command);
    
    if (!process.start(command))
    {
        setStatusMessage("Failed to start DeMucs");
        if (onProcessingComplete)
            onProcessingComplete(false, "Failed to start DeMucs process");
        return false;
    }
    
    // Store output for debugging
    juce::String processOutput;
    
    // Monitor the process with periodic progress updates
    int timeoutMs = 300000; // 5 minutes timeout
    int elapsedMs = 0;
    int checkIntervalMs = 2000; // Check every 2 seconds to slow down progress bar
    
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
            // Write all output to debug file for inspection
            juce::File outputDebugFile("/tmp/demucs_process_output.txt");
            outputDebugFile.replaceWithText(processOutput);
            
            // Look for progress indicators in the output
            if (currentOutput.contains("%|") || currentOutput.contains("seconds/s"))
            {
                setStatusMessage("DeMucs processing... " + currentOutput.substring(0, 50) + "...");
            }
            else if (currentOutput.contains("Selected model") || currentOutput.contains("Separated tracks"))
            {
                setStatusMessage("DeMucs: " + currentOutput.substring(0, 80) + "...");
            }
        }
        
        // Update progress based on elapsed time (rough estimation)
        double progress = 0.4 + (0.5 * static_cast<double>(elapsedMs) / timeoutMs);
        setProgress(juce::jmin(progress, 0.9));
        
        // Update status message periodically
        if (elapsedMs % 10000 == 0) // Every 10 seconds
        {
            setStatusMessage("Processing stems... (" + juce::String(elapsedMs / 1000) + "s elapsed)");
        }
        
        juce::Thread::sleep(checkIntervalMs);
        elapsedMs += checkIntervalMs;
    }
    
    if (process.isRunning())
    {
        process.kill();
        setStatusMessage("DeMucs process timed out");
        return false; // Timeout
    }
    
    int exitCode = process.getExitCode();
    
    // Read any remaining output
    auto remainingOutput = process.readAllProcessOutput();
    if (remainingOutput.isNotEmpty())
    {
        processOutput += remainingOutput;
    }
    
    setStatusMessage("DeMucs finished with exit code: " + juce::String(exitCode));
    
    // Store the output for later use in error reporting
    if (processOutput.isNotEmpty())
    {
        setStatusMessage("DeMucs output: " + processOutput.substring(0, 200) + "...");
        
        // Pass the output to the completion callback for better error reporting
        if (onProcessingComplete)
        {
            if (exitCode == 0)
                onProcessingComplete(true, "Stems have been successfully separated!");
            else
            {
                juce::String errorMsg = "DeMucs failed with exit code " + juce::String(exitCode) + ":\n\n";
                errorMsg += processOutput;
                onProcessingComplete(false, errorMsg);
            }
        }
        return exitCode == 0;
    }
    
    return exitCode == 0;
}