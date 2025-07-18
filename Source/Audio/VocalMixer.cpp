#include "VocalMixer.h"

VocalMixer::VocalMixer(const juce::File& recordingFile, const juce::File& karaokeFile, const juce::File& outputFile, int bufferSize)
    : Thread("VocalMixer"),
      recordingFile(recordingFile),
      karaokeFile(karaokeFile),
      outputFile(outputFile),
      bufferSizeForLatencyComp(bufferSize)
{
    updateProgress(0.0, "Initializing vocal mixer...");
}

VocalMixer::~VocalMixer()
{
}

void VocalMixer::run()
{
    updateProgress(0.1, "Checking FFmpeg availability...");
    
    if (!checkFFmpegAvailability())
    {
        if (onMixingComplete)
            onMixingComplete(false, "FFmpeg is not available. Please install FFmpeg:\n\n"
                                  "brew install ffmpeg\n\n"
                                  "FFmpeg is required to mix your vocals with the karaoke track.");
        return;
    }
    
    updateProgress(0.2, "Verifying input files...");
    
    // Check if input files exist
    if (!recordingFile.exists())
    {
        if (onMixingComplete)
            onMixingComplete(false, "Recording file not found: " + recordingFile.getFullPathName());
        return;
    }
    
    if (!karaokeFile.exists())
    {
        if (onMixingComplete)
            onMixingComplete(false, "Karaoke track not found: " + karaokeFile.getFullPathName());
        return;
    }
    
    updateProgress(0.3, "Preparing audio mixing...");
    
    // Trim audio files for latency compensation
    if (!trimAudioFilesForLatency())
    {
        return; // Error already reported in trimAudioFilesForLatency
    }
    
    // Create output directory if it doesn't exist
    auto outputDir = outputFile.getParentDirectory();
    if (!outputDir.exists())
    {
        if (!outputDir.createDirectory())
        {
            if (onMixingComplete)
                onMixingComplete(false, "Failed to create output directory: " + outputDir.getFullPathName());
            return;
        }
    }
    
    updateProgress(0.4, "Building mixing command...");
    
    juce::String command = buildMixingCommand();
    
    if (threadShouldExit())
        return;
    
    updateProgress(0.5, "Mixing vocals with karaoke track...");
    bool success = executeMixingCommand(command);
    
    if (threadShouldExit())
        return;
    
    if (success)
    {
        updateProgress(1.0, "Vocal mixing complete!");
        if (onMixingComplete)
            onMixingComplete(true, "Your vocals have been successfully mixed with the karaoke track!\n\nOutput: " + outputFile.getFullPathName());
    }
}

bool VocalMixer::checkFFmpegAvailability()
{
    juce::ChildProcess process;
    
    if (process.start("ffmpeg -version"))
    {
        process.waitForProcessToFinish(5000); // Wait up to 5 seconds
        return process.getExitCode() == 0;
    }
    
    return false;
}

juce::String VocalMixer::buildMixingCommand()
{
    juce::StringArray args;
    args.add("ffmpeg");
    
    // Input files
    args.add("-i"); args.add(recordingFile.getFullPathName());   // User vocals
    args.add("-i"); args.add(karaokeFile.getFullPathName());     // Karaoke track
    
    // Audio filter to mix the two inputs with volume adjustment
    // Convert mono vocals to stereo, maintain karaoke at full volume, vocals at full level
    args.add("-filter_complex");
    args.add("[0:a]volume=1.0,pan=stereo|c0=c0|c1=c0[vocals_stereo];[1:a]volume=1.0[karaoke];[vocals_stereo][karaoke]amix=inputs=2:duration=longest:dropout_transition=3,loudnorm=I=-13:LRA=11:TP=-1.5");
    
    // Output settings
    args.add("-c:a"); args.add("mp3");
    args.add("-b:a"); args.add("320k");
    args.add("-ac"); args.add("2"); // Force stereo output
    args.add("-y"); // Overwrite output file if it exists
    
    // Output file
    args.add(outputFile.getFullPathName());
    
    return args.joinIntoString(" ");
}

bool VocalMixer::executeMixingCommand(const juce::String& command)
{
    juce::ChildProcess process;
    
    updateProgress(0.6, "Starting FFmpeg mixing process...");
    
    if (!process.start(command))
    {
        updateProgress(0.6, "Failed to start FFmpeg");
        if (onMixingComplete)
            onMixingComplete(false, "Failed to start FFmpeg mixing process");
        return false;
    }
    
    // Monitor the process with progress updates
    int timeoutMs = 60000; // 1 minute timeout
    int elapsedMs = 0;
    int checkIntervalMs = 1000; // Check every 1 second
    
    while (process.isRunning() && elapsedMs < timeoutMs)
    {
        if (threadShouldExit())
        {
            process.kill();
            return false;
        }
        
        // Update progress based on elapsed time (rough estimation)
        double progress = 0.6 + (0.3 * static_cast<double>(elapsedMs) / timeoutMs);
        updateProgress(juce::jmin(progress, 0.9), "Mixing audio... (" + juce::String(elapsedMs / 1000) + "s)");
        
        juce::Thread::sleep(checkIntervalMs);
        elapsedMs += checkIntervalMs;
    }
    
    if (process.isRunning())
    {
        process.kill();
        updateProgress(0.9, "Mixing process timed out");
        if (onMixingComplete)
            onMixingComplete(false, "Audio mixing process timed out");
        return false;
    }
    
    int exitCode = process.getExitCode();
    
    updateProgress(0.95, "FFmpeg finished with exit code: " + juce::String(exitCode));
    
    if (exitCode != 0)
    {
        juce::String errorOutput = process.readAllProcessOutput();
        if (onMixingComplete)
        {
            juce::String errorMsg = "FFmpeg failed to mix audio (exit code " + juce::String(exitCode) + "):\n\n";
            errorMsg += errorOutput;
            onMixingComplete(false, errorMsg);
        }
        return false;
    }
    
    // Verify the output file was created
    if (!outputFile.exists())
    {
        if (onMixingComplete)
            onMixingComplete(false, "Output file was not created successfully");
        return false;
    }
    
    updateProgress(1.0, "Vocal mixing successful");
    return true;
}

void VocalMixer::updateProgress(double progress, const juce::String& message)
{
    if (onProgressUpdate)
    {
        juce::MessageManager::callAsync([this, progress, message]() {
            onProgressUpdate(progress, message);
        });
    }
}

bool VocalMixer::trimAudioFilesForLatency()
{
    // Get sample rate from the recording file using JUCE
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(recordingFile));
    if (!reader)
    {
        if (onMixingComplete)
            onMixingComplete(false, "Failed to read recording file metadata");
        return false;
    }
    
    double sampleRate = reader->sampleRate;
    
    // Create trimmed recording file (remove first 100ms)
    juce::File trimmedRecording = trimAudioFile(recordingFile, sampleRate);
    if (!trimmedRecording.exists())
    {
        if (onMixingComplete)
            onMixingComplete(false, "Failed to trim recording file for latency compensation");
        return false;
    }
    
    // Update file reference for mixing
    recordingFile = trimmedRecording;
    
    return true;
}

juce::File VocalMixer::trimAudioFile(const juce::File& inputFile, double sampleRate)
{
    juce::String suffix = "_trim100ms";
    
    juce::File outputFile = inputFile.getParentDirectory()
                              .getChildFile(inputFile.getFileNameWithoutExtension() + suffix + 
                                          "." + inputFile.getFileExtension());
    
    juce::StringArray args;
    args.add("ffmpeg");
    args.add("-i"); args.add(inputFile.getFullPathName());
    args.add("-af"); args.add("atrim=start=0.1"); // Trim first 100ms
    
    // Output settings - maintain same format as input
    args.add("-c:a"); args.add("pcm_s16le"); // Use PCM for precision
    args.add("-y"); // Overwrite
    args.add(outputFile.getFullPathName());
    
    juce::ChildProcess process;
    if (!process.start(args.joinIntoString(" ")))
    {
        return juce::File();
    }
    
    process.waitForProcessToFinish(30000); // 30 second timeout
    
    if (process.getExitCode() != 0 || !outputFile.exists())
    {
        return juce::File();
    }
    
    return outputFile;
}