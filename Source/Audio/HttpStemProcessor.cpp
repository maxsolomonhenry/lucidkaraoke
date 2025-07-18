#include "HttpStemProcessor.h"
#include "RVCProcessor.h"

HttpStemProcessor::HttpStemProcessor(const juce::File& initialInputFile, const juce::File& initialOutputDirectory, const juce::String& url,
                                   int retries, int delayMs, int maxDelay)
    : Thread("HttpStemProcessor"),
      inputFile(initialInputFile),
      outputDirectory(initialOutputDirectory),
      serviceUrl(url),
      maxRetries(retries),
      baseDelayMs(delayMs),
      maxDelayMs(maxDelay)
{
    updateProgress(0.0, "Initializing stem processor...");
}

HttpStemProcessor::~HttpStemProcessor()
{
}

void HttpStemProcessor::run()
{
    updateProgress(0.05, "Checking stem separation service...");
    
    // Check if service is available with retry
    if (!isServiceAvailableWithRetry())
    {
        if (onProcessingComplete)
            onProcessingComplete(false, "Stem separation service is not available after multiple attempts. Please check the service and try again.");
        return;
    }
    
    updateProgress(0.1, "Preparing audio file...");
    
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
    
    updateProgress(0.15, "Sending audio for processing...");
    
    // Send the separation request with retry
    if (!sendSeparationRequestWithRetry())
    {
        if (onProcessingComplete)
            onProcessingComplete(false, "Failed to process audio file after multiple attempts. Please check that the file format is supported.");
        return;
    }
    
    updateProgress(0.9, "Generating karaoke track...");
    
    // Generate karaoke track using existing logic
    if (!generateKaraokeTrack())
    {
        updateProgress(0.95, "Karaoke generation failed, but stems are available");
    }
    
    updateProgress(0.98, "Processing vocals with RVC...");
    
    // Process vocals with RVC (optional)
    if (processVocalWithRVC())
    {
        generateRVCKaraokeTrack();
    }
    
    updateProgress(1.0, "Stem separation completed!");
    
    if (onProcessingComplete)
        onProcessingComplete(true, "Stem separation completed successfully!");
}

bool HttpStemProcessor::isServiceAvailable()
{
    // Use curl with timeout for health check to avoid hanging
    juce::ChildProcess healthCheck;
    juce::String healthCommand = "curl -s --max-time 5 " + serviceUrl + "/health";
    
    if (!healthCheck.start(healthCommand))
        return false;
    
    bool finished = healthCheck.waitForProcessToFinish(6000); // 6 second timeout
    if (!finished)
    {
        healthCheck.kill();
        return false;
    }
    
    if (healthCheck.getExitCode() != 0)
        return false;
    
    juce::String response = healthCheck.readAllProcessOutput();
    return response.contains("healthy") || response.contains("status");
}

bool HttpStemProcessor::sendSeparationRequest()
{
    // For now, use a simplified approach with curl command
    // This avoids the complex multipart form handling in JUCE
    updateProgress(0.3, "Uploading audio file...");
    
    // Create temporary file path for output
    juce::File tempZip = outputDirectory.getChildFile("stems_temp.zip");

    // Ensure the output directory exists before we try to write to it.
    if (!outputDirectory.exists())
    {
        auto result = outputDirectory.createDirectory();
        if (result.failed())
        {
            if (onProcessingComplete)
                onProcessingComplete(false, "Failed to create temporary directory for stems. Error: " + result.getErrorMessage());
            return false;
        }
    }
    
    // Use curl to upload file and download result with proper quoting
    juce::StringArray curlArgs;
    curlArgs.add("curl");
    curlArgs.add("-v");
    curlArgs.add("-X");
    curlArgs.add("POST");
    curlArgs.add("-F");
    curlArgs.add("audio_file=@" + inputFile.getFullPathName());
    curlArgs.add("-F");
    curlArgs.add("format=mp3");
    curlArgs.add("-F");
    curlArgs.add("bitrate=320");
    curlArgs.add("-o");
    curlArgs.add(tempZip.getFullPathName());
    curlArgs.add(serviceUrl + "/separate");

    
    juce::String curlCommand = curlArgs.joinIntoString(" ");
    
    updateProgress(0.4, "Processing audio...");
    
    juce::ChildProcess curlProcess;
    if (!curlProcess.start(curlCommand))
    {
        updateProgress(0.45, "Failed to send request");
        return false;
    }
    
    // Monitor progress
    int timeout = 300000; // 5 minutes
    int elapsed = 0;
    int checkInterval = 2000; // Check every 2 seconds
    
    while (curlProcess.isRunning() && elapsed < timeout)
    {
        if (threadShouldExit())
        {
            curlProcess.kill();
            return false;
        }
        
        Thread::sleep(checkInterval);
        elapsed += checkInterval;
        
        // Update progress based on elapsed time (rough estimate)
        double processingProgress = 0.4 + (elapsed * 0.45 / timeout);
        updateProgress(processingProgress, "Processing audio... (" + juce::String(elapsed / 1000) + "s)");
    }
    
    if (curlProcess.isRunning())
    {
        curlProcess.kill();
        updateProgress(0.85, "Request timed out");
        return false;
    }
    
    int exitCode = curlProcess.getExitCode();
    juce::String output = curlProcess.readAllProcessOutput();

    // Log the output for debugging
    juce::Logger::writeToLog("cURL command: " + curlCommand);
    juce::Logger::writeToLog("cURL exit code: " + juce::String(exitCode));
    juce::Logger::writeToLog("cURL output (stdout/stderr): " + output);

    if (exitCode != 0)
    {
        updateProgress(0.87, "Request failed");
        return false;
    }
    
    updateProgress(0.85, "Downloading results...");
    
    // Check if we got a response file
    if (!tempZip.exists() || tempZip.getSize() == 0)
    {
        updateProgress(0.88, "No response received");
        return false;
    }
    
        updateProgress(0.88, "Extracting stems...");
    
    // Use juce::ZipFile to extract the downloaded archive
    bool success = extractStems(tempZip);
    tempZip.deleteFile(); // Clean up
    
    return success;
}

bool HttpStemProcessor::extractStems(const juce::File& zipFile)
{
    if (!zipFile.existsAsFile())
        return false;

    juce::ZipFile archive(zipFile);
    if (archive.getNumEntries() == 0)
        return false;

    auto result = archive.uncompressTo(outputDirectory);
    if (result.failed())
    {
        juce::Logger::writeToLog("Failed to extract zip file: " + result.getErrorMessage());
        return false;
    }

    return true;
}

bool HttpStemProcessor::downloadAndExtractStems(const juce::MemoryBlock& zipData)
{
    // This function is now deprecated, extraction is handled by extractStems.
    // Kept for compatibility in case it's called elsewhere, but should be removed in the future.
    juce::File tempZip = outputDirectory.getChildFile("stems_temp.zip");
    if (tempZip.replaceWithData(zipData.getData(), zipData.getSize()))
    {
        return extractStems(tempZip);
    }
    return false;
}

bool HttpStemProcessor::generateKaraokeTrack()
{
    // Use existing karaoke generation logic
    juce::File vocalsFile = outputDirectory.getChildFile("vocals.mp3");
    juce::File drumsFile = outputDirectory.getChildFile("drums.mp3");
    juce::File bassFile = outputDirectory.getChildFile("bass.mp3");
    juce::File otherFile = outputDirectory.getChildFile("other.mp3");
    juce::File karaokeFile = outputDirectory.getChildFile("karaoke.mp3");
    
    if (!drumsFile.exists() || !bassFile.exists() || !otherFile.exists())
    {
        return false;
    }
    
    // Use FFmpeg to mix non-vocal stems
    juce::StringArray ffmpegArgs;
    ffmpegArgs.add("ffmpeg");
    ffmpegArgs.add("-i"); ffmpegArgs.add(drumsFile.getFullPathName());
    ffmpegArgs.add("-i"); ffmpegArgs.add(bassFile.getFullPathName());
    ffmpegArgs.add("-i"); ffmpegArgs.add(otherFile.getFullPathName());
    ffmpegArgs.add("-filter_complex");
    ffmpegArgs.add("[0:a][1:a][2:a]amix=inputs=3:duration=longest:dropout_transition=0");
    ffmpegArgs.add("-y"); // Overwrite output file
    ffmpegArgs.add(karaokeFile.getFullPathName());
    
    juce::String ffmpegCommand = ffmpegArgs.joinIntoString(" ");
    
    juce::ChildProcess ffmpegProcess;
    if (!ffmpegProcess.start(ffmpegCommand))
    {
        return false;
    }
    
    return ffmpegProcess.waitForProcessToFinish(30000);
}

bool HttpStemProcessor::processVocalWithRVC()
{
    // Use existing RVC processing logic if available
    juce::File vocalsFile = outputDirectory.getChildFile("vocals.mp3");
    
    if (!vocalsFile.exists())
        return false;
    
    juce::File rvcOutputFile = outputDirectory.getChildFile("vocals_rvc.mp3");
    
    RVCProcessor rvcProcessor(vocalsFile, rvcOutputFile);
    rvcProcessor.startThread();
    
    // Wait for completion (simplified for now)
    while (rvcProcessor.isThreadRunning())
    {
        if (threadShouldExit())
        {
            rvcProcessor.stopThread(1000);
            return false;
        }
        Thread::sleep(100);
    }
    
    return rvcOutputFile.exists();
}

bool HttpStemProcessor::generateRVCKaraokeTrack()
{
    // Generate karaoke with RVC-processed vocals
    juce::File rvcVocalsFile = outputDirectory.getChildFile("vocals_rvc.mp3");
    juce::File drumsFile = outputDirectory.getChildFile("drums.mp3");
    juce::File bassFile = outputDirectory.getChildFile("bass.mp3");
    juce::File otherFile = outputDirectory.getChildFile("other.mp3");
    juce::File rvcKaraokeFile = outputDirectory.getChildFile("karaoke_with_rvc.mp3");
    
    if (!rvcVocalsFile.exists() || !drumsFile.exists() || !bassFile.exists() || !otherFile.exists())
    {
        return false;
    }
    
    // Use FFmpeg to mix all stems including RVC vocals
    juce::StringArray ffmpegArgs;
    ffmpegArgs.add("ffmpeg");
    ffmpegArgs.add("-i"); ffmpegArgs.add(rvcVocalsFile.getFullPathName());
    ffmpegArgs.add("-i"); ffmpegArgs.add(drumsFile.getFullPathName());
    ffmpegArgs.add("-i"); ffmpegArgs.add(bassFile.getFullPathName());
    ffmpegArgs.add("-i"); ffmpegArgs.add(otherFile.getFullPathName());
    ffmpegArgs.add("-filter_complex");
    ffmpegArgs.add("[0:a][1:a][2:a][3:a]amix=inputs=4:duration=longest:dropout_transition=0");
    ffmpegArgs.add("-y"); // Overwrite output file
    ffmpegArgs.add(rvcKaraokeFile.getFullPathName());
    
    juce::String ffmpegCommand = ffmpegArgs.joinIntoString(" ");
    
    juce::ChildProcess ffmpegProcess;
    if (!ffmpegProcess.start(ffmpegCommand))
    {
        return false;
    }
    
    return ffmpegProcess.waitForProcessToFinish(30000);
}

bool HttpStemProcessor::isTransientError(int exitCode, const juce::String& output)
{
    // Network-related curl exit codes that might indicate transient issues
    if (exitCode == 6 || exitCode == 7 || exitCode == 28) // Could not resolve host, couldn't connect, timeout
        return true;
    
    // Check for HTTP 5xx server errors in output (indicating server overload/cold start)
    if (output.contains("HTTP/1.1 5") || output.contains("HTTP/2 5"))
        return true;
    
    // Check for common transient error messages
    if (output.containsIgnoreCase("connection refused") ||
        output.containsIgnoreCase("connection timed out") ||
        output.containsIgnoreCase("temporarily unavailable") ||
        output.containsIgnoreCase("service unavailable") ||
        output.containsIgnoreCase("internal server error"))
        return true;
    
    return false;
}

void HttpStemProcessor::waitWithBackoff(int attemptNumber)
{
    // Exponential backoff with jitter: delay = baseDelay * 2^attempt + random(0, 1000)
    int delay = baseDelayMs * (1 << attemptNumber); // 2^attemptNumber
    delay = juce::jmin(delay, maxDelayMs); // Cap at maxDelay
    
    // Add jitter (random 0-1000ms) to avoid thundering herd
    delay += juce::Random::getSystemRandom().nextInt(1000);
    
    if (delay > 0)
    {
        updateProgress(0.05 + (attemptNumber * 0.02), "Waiting " + juce::String(delay / 1000.0, 1) + "s before retry...");
        Thread::sleep(delay);
    }
}

bool HttpStemProcessor::isServiceAvailableWithRetry()
{
    for (int attempt = 0; attempt <= maxRetries; ++attempt)
    {
        if (threadShouldExit())
            return false;
            
        if (attempt > 0)
        {
            updateProgress(0.05 + (attempt * 0.02), "Retrying service check... (attempt " + juce::String(attempt + 1) + "/" + juce::String(maxRetries + 1) + ")");
        }
        
        if (isServiceAvailable())
            return true;
        
        // If this was the last attempt, don't wait
        if (attempt < maxRetries)
        {
            waitWithBackoff(attempt);
        }
    }
    
    return false;
}

bool HttpStemProcessor::sendSeparationRequestWithRetry()
{
    for (int attempt = 0; attempt <= maxRetries; ++attempt)
    {
        if (threadShouldExit())
            return false;
            
        if (attempt > 0)
        {
            updateProgress(0.15 + (attempt * 0.02), "Retrying separation request... (attempt " + juce::String(attempt + 1) + "/" + juce::String(maxRetries + 1) + ")");
        }
        
        if (sendSeparationRequest())
            return true;
        
        // If this was the last attempt, don't wait
        if (attempt < maxRetries)
        {
            waitWithBackoff(attempt);
        }
    }
    
    return false;
}

void HttpStemProcessor::updateProgress(double progress, const juce::String& message)
{
    if (onProgressUpdate)
        onProgressUpdate(progress, message);
}