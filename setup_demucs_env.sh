#!/bin/bash

# setup_demucs_env.sh - Setup Python virtual environment for DeMucs

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VENV_PATH="$SCRIPT_DIR/demucs_env"

echo "Setting up DeMucs environment..."
echo "Script directory: $SCRIPT_DIR"
echo "Virtual environment path: $VENV_PATH"

# Check if Python 3 is available
if ! command -v python3 &> /dev/null; then
    echo "Error: python3 is not installed. Please install Python 3.8 or higher."
    exit 1
fi

# Check Python version (3.8 minimum for PyTorch)
PYTHON_VERSION=$(python3 -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')")
REQUIRED_VERSION="3.8"

if [[ "$(printf '%s\n' "$REQUIRED_VERSION" "$PYTHON_VERSION" | sort -V | head -n1)" != "$REQUIRED_VERSION" ]]; then
    echo "Error: Python 3.8 or higher is required. Found Python $PYTHON_VERSION"
    exit 1
fi

echo "Python $PYTHON_VERSION found - OK"

# Check if FFmpeg is available
if ! command -v ffmpeg &> /dev/null; then
    echo "Warning: FFmpeg is not installed. DeMucs requires FFmpeg for audio processing."
    echo "Please install FFmpeg:"
    echo "  macOS: brew install ffmpeg"
    echo "  Linux: sudo apt-get install ffmpeg"
    echo "  Windows: Download from https://ffmpeg.org/"
    echo ""
    read -p "Continue anyway? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Remove existing virtual environment if it exists
if [ -d "$VENV_PATH" ]; then
    echo "Removing existing virtual environment..."
    rm -rf "$VENV_PATH"
fi

# Create virtual environment
echo "Creating virtual environment..."
python3 -m venv "$VENV_PATH"

# Activate virtual environment
echo "Activating virtual environment..."
source "$VENV_PATH/bin/activate"

# Upgrade pip
echo "Upgrading pip..."
pip install --upgrade pip

# Install DeMucs and dependencies
echo "Installing DeMucs and dependencies..."
pip install demucs

# Install additional dependencies for RVC processing
echo "Installing additional audio processing dependencies..."
pip install torch torchaudio
pip install librosa soundfile
pip install numpy scipy

# Verify installation
echo "Verifying DeMucs installation..."
if python -m demucs --help > /dev/null 2>&1; then
    echo "✓ DeMucs installed successfully"
else
    echo "✗ DeMucs installation failed"
    exit 1
fi

# Test PyTorch
echo "Verifying PyTorch installation..."
if python -c "import torch; print(f'PyTorch {torch.__version__} installed successfully')" > /dev/null 2>&1; then
    echo "✓ PyTorch installed successfully"
else
    echo "✗ PyTorch installation failed"
    exit 1
fi

# Test audio libraries
echo "Verifying audio libraries..."
if python -c "import librosa, soundfile; print('Audio libraries installed successfully')" > /dev/null 2>&1; then
    echo "✓ Audio libraries installed successfully"
else
    echo "✗ Audio libraries installation failed"
    exit 1
fi

echo ""
echo "✓ Setup complete!"
echo ""
echo "Virtual environment created at: $VENV_PATH"
echo ""
echo "To manually activate the environment:"
echo "  source $VENV_PATH/bin/activate"
echo ""
echo "To test DeMucs manually:"
echo "  $VENV_PATH/bin/python -m demucs --help"
echo ""
echo "The LucidKaraoke application will now use this environment automatically."