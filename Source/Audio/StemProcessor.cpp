#include "StemProcessor.h"

StemProcessor::StemProcessor(const juce::File& inputFile, const juce::File& outputDirectory)
    : Thread("StemProcessor"),
      inputFile(inputFile),
      outputDirectory(outputDirectory)
{
    updateProgress(0.0, "Initializing DeMucs...");
}

StemProcessor::~StemProcessor()
{
}

void StemProcessor::run()
{
    updateProgress(0.1, "Checking DeMucs availability...");
    
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
    
    updateProgress(0.15, "Checking FFmpeg availability...");
    
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
    
    updateProgress(0.2, "Preparing audio file...");
    
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
    
    updateProgress(0.3, "Running DeMucs stem separation...");
    
    juce::String command = buildDeMucsCommand();
    
    if (threadShouldExit())
        return;
    
    updateProgress(0.4, "Processing audio...");
    bool success = executeDeMucsCommand(command);
    
    if (threadShouldExit())
        return;
    
    if (success)
    {
        updateProgress(0.9, "Generating karaoke track...");
        success = generateKaraokeTrack();
    }
    
    updateProgress(1.0, "Processing complete!");
    
    // Error handling is now done in executeDeMucsCommand
    // Only call completion callback if it wasn't already called
    if (onProcessingComplete && success)
    {
        onProcessingComplete(true, "Stems and karaoke track have been successfully generated!");
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
    
    updateProgress(0.35, "Starting DeMucs...");
    
    // Write the command to a debug file so we can see exactly what's being executed
    juce::File debugFile("/tmp/demucs_command.txt");
    debugFile.replaceWithText(command);
    
    if (!process.start(command))
    {
        updateProgress(0.4, "Failed to start DeMucs");
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
                updateProgress(0.4 + (0.4 * static_cast<double>(elapsedMs) / timeoutMs), "DeMucs processing... " + currentOutput.substring(0, 50) + "...");
            }
            else if (currentOutput.contains("Selected model") || currentOutput.contains("Separated tracks"))
            {
                updateProgress(0.4 + (0.4 * static_cast<double>(elapsedMs) / timeoutMs), "DeMucs: " + currentOutput.substring(0, 80) + "...");
            }
        }
        
        // Update progress based on elapsed time (rough estimation)
        double progress = 0.4 + (0.5 * static_cast<double>(elapsedMs) / timeoutMs);
        
        // Update status message periodically
        if (elapsedMs % 10000 == 0) // Every 10 seconds
        {
            updateProgress(juce::jmin(progress, 0.9), "Processing stems... (" + juce::String(elapsedMs / 1000) + "s elapsed)");
        }
        
        juce::Thread::sleep(checkIntervalMs);
        elapsedMs += checkIntervalMs;
    }
    
    if (process.isRunning())
    {
        process.kill();
        updateProgress(0.9, "DeMucs process timed out");
        return false; // Timeout
    }
    
    int exitCode = process.getExitCode();
    
    // Read any remaining output
    auto remainingOutput = process.readAllProcessOutput();
    if (remainingOutput.isNotEmpty())
    {
        processOutput += remainingOutput;
    }
    
    updateProgress(0.95, "DeMucs finished with exit code: " + juce::String(exitCode));
    
    // Store the output for later use in error reporting
    if (processOutput.isNotEmpty())
    {
        updateProgress(1.0, "DeMucs output: " + processOutput.substring(0, 200) + "...");
        
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

bool StemProcessor::generateKaraokeTrack()
{
    // Find the DeMucs output directory (should be outputDirectory/htdemucs_ft/inputFileName)
    juce::String inputFileName = inputFile.getFileNameWithoutExtension();
    juce::File stemsDir = outputDirectory.getChildFile("htdemucs_ft").getChildFile(inputFileName);
    
    if (!stemsDir.exists())
    {
        if (onProcessingComplete)
            onProcessingComplete(false, "Could not find stems directory: " + stemsDir.getFullPathName());
        return false;
    }
    
    // Check if all required stems exist
    juce::File drumsFile = stemsDir.getChildFile("drums.mp3");
    juce::File bassFile = stemsDir.getChildFile("bass.mp3");
    juce::File otherFile = stemsDir.getChildFile("other.mp3");
    juce::File karaokeFile = stemsDir.getChildFile("karaoke.mp3");
    
    if (!drumsFile.exists() || !bassFile.exists() || !otherFile.exists())
    {
        if (onProcessingComplete)
            onProcessingComplete(false, "Missing required stem files in: " + stemsDir.getFullPathName());
        return false;
    }
    
    // Use FFmpeg to mix the non-vocal stems into a karaoke track
    juce::StringArray args;
    args.add("ffmpeg");
    args.add("-i"); args.add(drumsFile.getFullPathName());
    args.add("-i"); args.add(bassFile.getFullPathName());
    args.add("-i"); args.add(otherFile.getFullPathName());
    args.add("-filter_complex");
    args.add("[0:a][1:a][2:a]amix=inputs=3:duration=longest:dropout_transition=3");
    args.add("-c:a"); args.add("mp3");
    args.add("-b:a"); args.add("320k");
    args.add("-y"); // Overwrite output file if it exists
    args.add(karaokeFile.getFullPathName());
    
    juce::String command = args.joinIntoString(" ");
    
    juce::ChildProcess process;
    if (!process.start(command))
    {
        if (onProcessingComplete)
            onProcessingComplete(false, "Failed to start FFmpeg for karaoke generation");
        return false;
    }
    
    // Wait for FFmpeg to complete
    process.waitForProcessToFinish(60000); // 1 minute timeout
    
    int exitCode = process.getExitCode();
    if (exitCode != 0)
    {
        juce::String errorOutput = process.readAllProcessOutput();
        if (onProcessingComplete)
            onProcessingComplete(false, "FFmpeg failed to generate karaoke track (exit code " + 
                               juce::String(exitCode) + "): " + errorOutput);
        return false;
    }
    
    // Verify the karaoke file was created
    if (!karaokeFile.exists())
    {
        if (onProcessingComplete)
            onProcessingComplete(false, "Karaoke file was not created successfully");
        return false;
    }
    
    updateProgress(0.95, "Karaoke track generated successfully");
    return true;
}

void StemProcessor::updateProgress(double progress, const juce::String& message)
{
    if (onProgressUpdate)
    {
        juce::MessageManager::callAsync([this, progress, message]() {
            onProgressUpdate(progress, message);
        });
    }
}