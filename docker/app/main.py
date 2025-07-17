#!/usr/bin/env python3
"""
DeMucs Stem Separation HTTP Service
Simple wrapper around DeMucs CLI with pre-downloaded weights
"""

import os
import tempfile
import shutil
import zipfile
import subprocess
from pathlib import Path
from typing import Optional

from fastapi import FastAPI, File, UploadFile, HTTPException, BackgroundTasks
from fastapi.responses import FileResponse
from fastapi.middleware.cors import CORSMiddleware
import uvicorn

def is_gpu_available() -> bool:
    """Check if CUDA GPU is available for DeMucs"""
    try:
        import torch
        return torch.cuda.is_available()
    except ImportError:
        # Fallback: check nvidia-smi command
        try:
            result = subprocess.run(["nvidia-smi"], capture_output=True, timeout=5)
            return result.returncode == 0
        except:
            return False

app = FastAPI(
    title="DeMucs Stem Separation Service",
    description="HTTP API for AI-powered audio stem separation using DeMucs CLI",
    version="1.0.0"
)

# Add CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.get("/health")
async def health_check():
    """Health check endpoint"""
    # Check if DeMucs is available
    try:
        result = subprocess.run(["python", "-m", "demucs", "--help"], 
                              capture_output=True, text=True, timeout=10)
        demucs_available = result.returncode == 0
    except:
        demucs_available = False
    
    return {
        "status": "healthy" if demucs_available else "unhealthy",
        "demucs_available": demucs_available,
        "gpu_available": is_gpu_available(),
        "model": "htdemucs_ft"
    }

@app.get("/models")
async def list_models():
    """List available DeMucs models"""
    return {
        "available_models": ["htdemucs_ft"],
        "current_model": "htdemucs_ft"
    }

@app.post("/separate")
async def separate_stems(
    background_tasks: BackgroundTasks,
    audio_file: UploadFile = File(...),
    model: Optional[str] = "htdemucs_ft",
    format: Optional[str] = "mp3",
    bitrate: Optional[int] = 320
):
    """
    Separate audio stems from uploaded file using DeMucs CLI
    """
    if not audio_file.filename:
        raise HTTPException(status_code=400, detail="No file provided")
    
    # Validate file type
    allowed_extensions = {'.mp3', '.wav', '.flac', '.m4a', '.aac', '.ogg'}
    file_ext = Path(audio_file.filename).suffix.lower()
    if file_ext not in allowed_extensions:
        raise HTTPException(
            status_code=400, 
            detail=f"Unsupported file format: {file_ext}. Supported: {', '.join(allowed_extensions)}"
        )
    
    # Create temporary directories
    temp_dir = tempfile.mkdtemp(prefix="demucs_")
    input_path = None
    output_zip = None
    
    try:
        # Save uploaded file
        input_path = Path(temp_dir) / f"input{file_ext}"
        with open(input_path, "wb") as f:
            content = await audio_file.read()
            f.write(content)
        
        # Run DeMucs CLI command
        output_dir = Path(temp_dir) / "output"
        output_dir.mkdir(exist_ok=True)
        
        # Build DeMucs command
        cmd = [
            "python", "-m", "demucs",
            "--mp3",
            "--mp3-bitrate", str(bitrate),
            "-n", model,
            "-o", str(output_dir)
        ]
        
        # Add GPU support if available
        if is_gpu_available():
            cmd.extend(["--device", "cuda"])
        
        cmd.append(str(input_path))
        
        # Run the command
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=300  # 5 minute timeout
        )
        
        if result.returncode != 0:
            raise Exception(f"DeMucs failed: {result.stderr}")
        
        # Find the generated stems directory
        stems_dir = output_dir / model / input_path.stem
        if not stems_dir.exists():
            raise Exception("Stems directory not found")
        
        # Create ZIP file with stems
        output_zip = Path(temp_dir) / f"{Path(audio_file.filename).stem}_stems.zip"
        with zipfile.ZipFile(output_zip, 'w', zipfile.ZIP_DEFLATED) as zipf:
            for stem_file in stems_dir.glob("*.mp3"):
                zipf.write(stem_file, stem_file.name)
        
        # Schedule cleanup
        background_tasks.add_task(cleanup_temp_dir, temp_dir)
        
        return FileResponse(
            path=str(output_zip),
            filename=f"{Path(audio_file.filename).stem}_stems.zip",
            media_type="application/zip"
        )
        
    except subprocess.TimeoutExpired:
        # Cleanup on timeout
        if temp_dir and Path(temp_dir).exists():
            shutil.rmtree(temp_dir, ignore_errors=True)
        raise HTTPException(status_code=504, detail="Processing timeout")
        
    except Exception as e:
        # Cleanup on error
        if temp_dir and Path(temp_dir).exists():
            shutil.rmtree(temp_dir, ignore_errors=True)
        raise HTTPException(status_code=500, detail=f"Stem separation failed: {str(e)}")

def cleanup_temp_dir(temp_dir: str):
    """Background task to cleanup temporary directory"""
    try:
        if Path(temp_dir).exists():
            shutil.rmtree(temp_dir)
    except Exception:
        pass  # Ignore cleanup errors

@app.get("/")
async def root():
    """Root endpoint with service information"""
    return {
        "service": "DeMucs Stem Separation API",
        "version": "1.0.0",
        "endpoints": {
            "POST /separate": "Separate audio stems",
            "GET /health": "Health check",
            "GET /models": "List available models"
        }
    }

if __name__ == "__main__":
    # Configure based on environment
    host = os.getenv("HOST", "0.0.0.0")
    port = int(os.getenv("PORT", 8000))
    workers = int(os.getenv("WORKERS", 1))
    
    uvicorn.run(
        "main:app",
        host=host,
        port=port,
        workers=workers,
        access_log=True,
        log_level="info"
    )