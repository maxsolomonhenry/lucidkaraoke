# 🎤 LucidKaraoke

**Transform any song into your personal karaoke experience with AI-powered voice conversion**

LucidKaraoke revolutionizes karaoke by letting you sing along to *any* song and get back a professional recording with your voice replacing the original vocals. Using cutting-edge AI stem separation and voice conversion technology, it extracts the instrumental track, records your performance, and seamlessly blends your voice into the mix.

## 🚀 Current Progress

### ✅ Completed Features
- [x] **Audio Engine**: JUCE-based cross-platform audio processing
- [x] **File Loading**: Support for standard audio formats
- [x] **Waveform Visualization**: Real-time audio display with playhead
- [x] **Transport Controls**: Play, pause, stop, and seek functionality
- [x] **Local Stem Separation**: DeMucs integration for vocals/instrumental splitting
- [x] **Dark Theme UI**: Modern, professional interface design
- [x] **Multi-format Export**: VST3, AU, and Standalone builds

### 🔧 In Development
- [ ] **Cloud-Based Processing**: Move stem separation to cloud for faster processing
- [ ] **Voice Conversion Pipeline**: AI model training and vocal replacement
- [ ] **Cloud Voice Processing**: Python script integration via API calls
- [ ] **UX Overhaul**: Streamlined workflow and improved user experience
- [ ] **Portable Setup**: One-click installation and configuration
- [ ] **Real-time Monitoring**: Live audio feedback during recording
- [ ] **Batch Processing**: Handle multiple songs efficiently

![alt text](figs/screenshot.png)

## 🎯 Vision

Imagine being able to:
- **Sing with your favorite artists** - Load any song and perform alongside the music
- **Create professional covers** - Get studio-quality recordings with your voice
- **Experiment with styles** - Try singing in different genres and vocal styles
- **Share your performances** - Export polished tracks ready for social media

LucidKaraoke makes this possible by combining advanced AI with an intuitive interface, putting professional-grade vocal production tools in everyone's hands.

## 🛠️ Technical Stack

- **Audio Framework**: JUCE (cross-platform C++)
- **Stem Separation**: DeMucs (Python-based AI)
- **Voice Conversion**: Custom AI pipeline (in development)
- **Build System**: CMake
- **UI Theme**: Custom dark theme implementation

## 📋 Prerequisites

- CMake 3.22+
- C++17 compiler
- Python 3.x with DeMucs:
  ```bash
  pip install demucs
  brew install ffmpeg  # macOS
  ```
- JUCE framework (located at `../JUCE` relative to project root)

## 🏗️ Building

```bash
# Configure and build
mkdir build && cd build
cmake ..
cmake --build .

# Build specific targets
cmake --build . --target lucidkaraoke_Standalone    # Standalone app
cmake --build . --target lucidkaraoke_VST3          # VST3 plugin
cmake --build . --target lucidkaraoke_AU            # Audio Unit plugin
```

### Build Output
- **Standalone**: `build/lucidkaraoke_artefacts/Standalone/LucidKaraoke.app`
- **VST3**: `build/lucidkaraoke_artefacts/VST3/LucidKaraoke.vst3`
- **AU**: `build/lucidkaraoke_artefacts/AU/LucidKaraoke.component`

## 🚀 Quick Start

1. **Build the application** using the instructions above
2. **Load an audio file** using the Load button
3. **View the waveform** and use transport controls to navigate
4. **Split stems** using the Split button (separates vocals from instrumentals)
5. **Record your performance** (coming soon!)

## 🔮 Roadmap

### Phase 1: Core Foundation ✅
- Basic audio engine and UI components
- Local stem separation capability
- File I/O and waveform visualization

### Phase 2: Cloud Integration 🔧
- Migrate processing to cloud infrastructure
- API-based voice conversion pipeline
- Improved performance and scalability

### Phase 3: User Experience 📅
- Streamlined workflow design
- Real-time audio monitoring
- One-click setup and deployment

### Phase 4: Advanced Features 📅
- Multiple voice style options
- Batch processing capabilities
- Social sharing integration

## 🤝 Contributing

This is an active development project. The codebase is being optimized for rapid prototyping and easy setup across different development environments.

## 📄 License

[License information to be added]

---

*LucidKaraoke - Where your voice meets any song* 🎵