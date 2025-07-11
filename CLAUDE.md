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
- Python 3.x with DeMucs for stem separation:
  ```bash
  pip install demucs
  brew install ffmpeg  # macOS
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