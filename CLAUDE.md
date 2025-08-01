# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

LucidKaraoke is a JUCE-based audio plugin for stem separation and karaoke applications. It provides audio playback, waveform visualization, and AI-powered stem separation using DeMucs.

## Build System

This project uses CMake with JUCE framework:

### Prerequisites
- CMake 3.22+
- JUCE framework (located at `../JUCE` relative to project root)
- C++17 compiler
- Docker and Docker Compose (for stem separation service)

### Build Commands
```bash
# Configure and build (Release - default)
mkdir build && cd build
cmake ..
cmake --build .

# Configure and build (Debug - enables DBG macro and debugging features)
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Build specific targets
cmake --build . --target lucidkaraoke_Standalone    # Standalone application
cmake --build . --target lucidkaraoke_VST3          # VST3 plugin
cmake --build . --target lucidkaraoke_AU            # Audio Unit plugin
```

### Debug Build Features
When building with `CMAKE_BUILD_TYPE=Debug`, the following JUCE debugging features are enabled:
- `JUCE_ENABLE_REPAINT_DEBUGGING=1`: Enables the DBG macro for debug output
- `JUCE_LOG_ASSERTIONS=1`: Enables assertion logging
- `JUCE_CHECK_MEMORY_LEAKS=1`: Enables memory leak detection

### Build Output
- Standalone: `build/lucidkaraoke_artefacts/Standalone/LucidKaraoke.app`
- VST3: `build/lucidkaraoke_artefacts/VST3/LucidKaraoke.vst3`
- AU: `build/lucidkaraoke_artefacts/AU/LucidKaraoke.component`

## Architecture

### Core Components
- **AudioProcessor** (`Source/PluginProcessor.*`): Main audio processing engine with transport controls
- **AudioProcessorEditor** (`Source/PluginEditor.*`): Main UI container with dark theme
- **HttpStemProcessor** (`Source/Audio/HttpStemProcessor.*`): Handles cloud-based stem separation via HTTP API with retry logic
- **RVCProcessor** (`Source/Audio/RVCProcessor.*`): Voice conversion processor (placeholder implementation)
- **VocalMixer** (`Source/Audio/VocalMixer.*`): Audio mixing and blending capabilities
- **WaveformDisplay** (`Source/Components/WaveformDisplay.*`): Audio visualization with playhead and seeking
- **TransportControls** (`Source/Components/TransportControls.*`): Play/pause/stop controls
- **LoadButton** (`Source/Components/LoadButton.*`): File loading interface
- **SplitButton** (`Source/Components/SplitButton.*`): Stem separation trigger
- **RecordButton** (`Source/Components/RecordButton.*`): Audio recording interface
- **ProgressBar** (`Source/Components/ProgressBar.*`): Progress visualization for long operations
- **SourceToggleButton** (`Source/Components/SourceToggleButton.*`): Audio source switching controls
- **DarkTheme** (`Source/LookAndFeel/DarkTheme.*`): Custom UI styling
- **Config** (`Source/Config/config.h`): Configuration management and settings

### Key Design Patterns
- JUCE component-based architecture with proper RAII
- Observer pattern for audio state changes (ChangeListener)
- Threaded processing for stem separation (ThreadWithProgressWindow)
- Function callbacks for component communication

### External Dependencies
- **Cloud Stem Service**: HTTP-based DeMucs service for AI stem separation
- **JUCE**: Audio framework for cross-platform plugin development

## Development Notes

### Stem Separation Workflow
1. User loads audio file via LoadButton
2. WaveformDisplay shows audio thumbnail with playhead
3. SplitButton triggers HttpStemProcessor in background thread
4. HttpStemProcessor uploads audio to cloud service with automatic retry logic for cold starts
5. Cloud service processes audio using DeMucs and returns stem files
6. Output stems downloaded and saved to user-specified directory

### Audio Transport State
The AudioProcessor maintains three states: Stopped, Playing, Paused. State changes are broadcast to UI components via ChangeListener pattern.

### File I/O
- Supports standard audio formats via JUCE AudioFormatManager
- Files loaded through juce::URL for cross-platform compatibility
- Stem processing output organized in separate directories per track

### Cross-Platform Compatibility
- **Shell Scripts**: ALWAYS use Unix line endings (LF) for macOS/Linux compatibility. When creating new shell scripts (.sh files), ensure they use LF line endings to prevent "bad interpreter" errors. If line ending issues occur, fix with:
  ```bash
  sed -i '' 's/\r$//' script_name.sh
  ```
  **IMPORTANT**: All shell scripts must be created with LF line endings from the start. Use your editor's line ending settings or run the sed command immediately after creating any .sh file.
- **Docker Service**: Stem separation is handled by containerized DeMucs service via HTTP API

## Docker Integration

### DeMucs Docker Service
The project now includes a containerized DeMucs service for stem separation:

- **Location**: `docker/` directory contains all Docker-related files
- **Service**: FastAPI HTTP service running DeMucs with pre-downloaded model weights
- **Integration**: `HttpStemProcessor` class provides C++ integration with automatic retry logic
- **GPU Support**: Automatic CUDA detection with CPU fallback
- **Cloud Ready**: Compatible with Google Cloud Functions/Cloud Run
- **URL Configuration**: Service requires URL configuration to connect to remote cloud service for DeMucs processing and future voice cloning capabilities

### Docker Commands
```bash
# Build the container
cd docker && ./build.sh

# Start CPU-only service
docker compose --profile cpu up -d

# Start GPU service (requires NVIDIA Docker)
docker compose --profile gpu up -d

# Test the service
./test_service.sh
```

## Build Tips
- You can cmake --build with the -j flag to parallelize it please