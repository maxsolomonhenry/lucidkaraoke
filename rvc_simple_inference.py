#!/usr/bin/env python3
"""
Simple RVC inference script for LucidKaraoke
This script performs basic voice conversion using minimal RVC components
"""

import argparse
import os
import sys
import numpy as np
import soundfile as sf
import librosa
import torch
import torchcrepe
from scipy.signal import savgol_filter

def extract_f0_crepe(audio, sr, hop_length=512):
    """Extract F0 using CREPE"""
    print("Extracting pitch using CREPE...")
    
    # CREPE expects audio at 16kHz
    if sr != 16000:
        audio_16k = librosa.resample(audio, orig_sr=sr, target_sr=16000)
    else:
        audio_16k = audio
    
    # Convert to torch tensor and ensure correct shape
    if not isinstance(audio_16k, torch.Tensor):
        audio_16k = torch.from_numpy(audio_16k.astype(np.float32))
    
    # Ensure audio is 1D
    if len(audio_16k.shape) > 1:
        audio_16k = audio_16k.squeeze()
    
    # Extract pitch using torchcrepe
    try:
        pitch = torchcrepe.predict(
            audio_16k,
            sample_rate=16000,
            hop_length=160,  # 10ms hop at 16kHz
            fmin=50,
            fmax=550,
            model='tiny',
            device='mps' if torch.backends.mps.is_available() else 'cpu'
        )
    except Exception as e:
        print(f"CREPE failed: {e}, falling back to basic pitch tracking")
        # Fallback to librosa if CREPE fails
        if sr != 16000:
            audio_for_yin = librosa.resample(audio, orig_sr=sr, target_sr=16000)
        else:
            audio_for_yin = audio
        pitch = librosa.yin(audio_for_yin, fmin=50, fmax=550, sr=16000)
        return pitch
    
    # Convert back to original sample rate timing
    if sr != 16000:
        target_length = len(audio) // hop_length + 1
        pitch = np.interp(
            np.linspace(0, len(pitch) - 1, target_length),
            np.arange(len(pitch)),
            pitch
        )
    
    return pitch

def apply_pitch_shift(f0, semitones):
    """Apply pitch shift to F0"""
    if semitones == 0:
        return f0
    
    # Convert semitones to frequency ratio
    ratio = 2 ** (semitones / 12.0)
    return f0 * ratio

def simple_voice_conversion(input_path, output_path, pitch_shift=0, f0_method="crepe"):
    """
    Perform simple voice conversion
    For now, this is a placeholder that applies pitch shifting and basic processing
    In a full implementation, this would load and use RVC models
    """
    print(f"Loading audio from: {input_path}")
    
    # Load audio
    audio, sr = librosa.load(input_path, sr=None)
    print(f"Audio loaded: {len(audio)} samples at {sr}Hz")
    
    # Extract F0
    if f0_method == "crepe":
        f0 = extract_f0_crepe(audio, sr)
    else:
        # Fallback to basic pitch tracking
        f0 = librosa.yin(audio, fmin=50, fmax=550)
    
    print(f"F0 extracted: {len(f0)} frames")
    
    # Apply pitch shift
    if pitch_shift != 0:
        print(f"Applying pitch shift: {pitch_shift} semitones")
        f0_shifted = apply_pitch_shift(f0, pitch_shift)
        
        # Use PSOLA or simple time-stretching for pitch shifting
        # This is a basic implementation - a full RVC would use neural vocoders
        audio_shifted = librosa.effects.pitch_shift(audio, sr=sr, n_steps=pitch_shift)
    else:
        audio_shifted = audio
    
    # Apply some basic filtering to simulate voice conversion
    print("Applying voice processing...")
    
    # Add slight formant shifting effect
    if pitch_shift != 0:
        # Simple spectral envelope modification
        stft = librosa.stft(audio_shifted)
        magnitude = np.abs(stft)
        phase = np.angle(stft)
        
        # Apply smoothing to simulate voice character change
        for i in range(magnitude.shape[0]):
            magnitude[i] = savgol_filter(magnitude[i], 5, 2)
        
        # Reconstruct audio
        stft_modified = magnitude * np.exp(1j * phase)
        audio_converted = librosa.istft(stft_modified)
    else:
        audio_converted = audio_shifted
    
    # Normalize audio
    audio_converted = audio_converted / np.max(np.abs(audio_converted)) * 0.9
    
    print(f"Saving converted audio to: {output_path}")
    sf.write(output_path, audio_converted, sr)
    
    print("Voice conversion completed successfully!")
    return True

def main():
    parser = argparse.ArgumentParser(description="Simple RVC inference for LucidKaraoke")
    parser.add_argument("--input", required=True, help="Input vocal file")
    parser.add_argument("--output", required=True, help="Output file")
    parser.add_argument("--model", help="RVC model path (currently unused)")
    parser.add_argument("--f0_method", default="crepe", help="F0 extraction method")
    parser.add_argument("--pitch", type=float, default=0, help="Pitch shift in semitones")
    parser.add_argument("--quality", type=int, default=128, help="Quality setting (currently unused)")
    
    args = parser.parse_args()
    
    # Validate input file
    if not os.path.exists(args.input):
        print(f"Error: Input file not found: {args.input}")
        sys.exit(1)
    
    # Create output directory if needed
    os.makedirs(os.path.dirname(args.output), exist_ok=True)
    
    try:
        success = simple_voice_conversion(
            args.input, 
            args.output, 
            pitch_shift=args.pitch,
            f0_method=args.f0_method
        )
        
        if success:
            print("RVC inference completed successfully!")
            sys.exit(0)
        else:
            print("RVC inference failed!")
            sys.exit(1)
            
    except Exception as e:
        print(f"Error during voice conversion: {str(e)}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == "__main__":
    main()