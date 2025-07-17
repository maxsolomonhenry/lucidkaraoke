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
- Python 3.8+ and FFmpeg for stem separation:
  ```bash
  # Run the setup script to create virtual environment with DeMucs
  ./setup_demucs_env.sh
  
  # Or install manually:
  brew install ffmpeg  # macOS (required)
  python3 -m venv demucs_env
  source demucs_env/bin/activate
  pip install demucs torch librosa soundfile
  ```

### Build Commands
```bash
# Configure and build
mkdir build && cd build
cmake ..
cmake --build .

# Build specific targets
cmake --build . --target lucidkaraoke_Standalone    # Standalone application
cmake --build . --target lucidkaraoke_VST3          # VST3 plugin
cmake --build . --target lucidkaraoke_AU            # Audio Unit plugin
```

### Build Output
- Standalone: `build/lucidkaraoke_artefacts/Standalone/LucidKaraoke.app`
- VST3: `build/lucidkaraoke_artefacts/VST3/LucidKaraoke.vst3`
- AU: `build/lucidkaraoke_artefacts/AU/LucidKaraoke.component`

## Architecture

### Core Components
- **AudioProcessor** (`Source/PluginProcessor.*`): Main audio processing engine with transport controls
- **AudioProcessorEditor** (`Source/PluginEditor.*`): Main UI container with dark theme
- **StemProcessor** (`Source/Audio/StemProcessor.*`): Handles DeMucs integration for stem separation
- **WaveformDisplay** (`Source/Components/WaveformDisplay.*`): Audio visualization with playhead and seeking
- **TransportControls** (`Source/Components/TransportControls.*`): Play/pause/stop controls
- **LoadButton** (`Source/Components/LoadButton.*`): File loading interface
- **SplitButton** (`Source/Components/SplitButton.*`): Stem separation trigger
- **DarkTheme** (`Source/LookAndFeel/DarkTheme.*`): Custom UI styling

### Key Design Patterns
- JUCE component-based architecture with proper RAII
- Observer pattern for audio state changes (ChangeListener)
- Threaded processing for stem separation (ThreadWithProgressWindow)
- Function callbacks for component communication

### External Dependencies
- **DeMucs**: Python-based AI stem separation (called via subprocess)
- **FFmpeg**: Audio format conversion (required by DeMucs)
- **JUCE**: Audio framework for cross-platform plugin development

## Development Notes

### Stem Separation Workflow
1. User loads audio file via LoadButton
2. WaveformDisplay shows audio thumbnail with playhead
3. SplitButton triggers StemProcessor in background thread
4. StemProcessor calls DeMucs via system command
5. Output stems saved to user-specified directory

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
- **Python Environment**: Code uses relative paths to find `demucs_env/` virtual environment, checking both executable-relative (`../demucs_env/`) and working directory-relative (`demucs_env/`) locations

## Docker Integration

### DeMucs Docker Service
The project now includes a containerized DeMucs service for stem separation:

- **Location**: `docker/` directory contains all Docker-related files
- **Service**: FastAPI HTTP service running DeMucs with pre-downloaded model weights
- **Integration**: `HttpStemProcessor` and `DockerManager` classes provide C++ integration
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