# DeMucs Docker Service

This directory contains the Docker configuration for running DeMucs stem separation as a containerized HTTP service.

## Overview

The Docker service provides a REST API for audio stem separation using DeMucs, eliminating the need for local Python environment setup. The container includes pre-downloaded model weights to avoid cold start delays.

## Features

- **Pre-loaded Models**: Contains `htdemucs_ft` model weights
- **GPU Support**: Automatic CUDA detection with CPU fallback
- **HTTP API**: RESTful interface for stem separation
- **Multi-format Support**: Handles various audio formats (MP3, WAV, FLAC, etc.)
- **Health Monitoring**: Built-in health checks and status endpoints

## Quick Start

### 1. Build the Container

```bash
cd docker
./build.sh
```

**Note**: The first build takes 10-15 minutes as it downloads the 3.4GB PyTorch base image and pre-downloads DeMucs model weights. Subsequent builds are much faster due to Docker layer caching.

### 2. Start the Service

**CPU-only mode:**
```bash
docker compose --profile cpu up -d
```

**GPU mode (requires NVIDIA Docker runtime):**
```bash
docker compose --profile gpu up -d
```

**Development mode (with code hot-reloading):**
```bash
docker compose --profile dev up -d
```

### 3. Test the Service

```bash
# Health check
curl http://localhost:8000/health

# List available models
curl http://localhost:8000/models

# Separate stems (example)
curl -X POST -F "audio_file=@/path/to/audio.mp3" \
     http://localhost:8000/separate \
     --output stems.zip
```

## API Endpoints

### `GET /health`
Returns service health status and configuration.

**Response:**
```json
{
  "status": "healthy",
  "cuda_available": true,
  "device": "cuda:0",
  "model_loaded": true
}
```

### `GET /models`
Lists available DeMucs models.

**Response:**
```json
{
  "available_models": ["htdemucs_ft"],
  "current_model": "htdemucs_ft",
  "device": "cuda:0"
}
```

### `POST /separate`
Separates audio stems from uploaded file.

**Parameters:**
- `audio_file` (file, required): Audio file to process
- `model` (string, optional): DeMucs model to use (default: "htdemucs_ft")
- `format` (string, optional): Output format - mp3, wav, flac (default: "mp3")
- `bitrate` (integer, optional): Audio bitrate for compressed formats (default: 320)

**Response:**
ZIP file containing separated stems:
- `vocals.mp3` - isolated vocals
- `drums.mp3` - drum tracks
- `bass.mp3` - bass lines
- `other.mp3` - remaining instruments

## Configuration

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `HOST` | `0.0.0.0` | Service bind address |
| `PORT` | `8000` | Service port |
| `WORKERS` | `1` | Number of worker processes |
| `CUDA_VISIBLE_DEVICES` | (auto) | GPU device selection |

### Docker Compose Profiles

| Profile | Port | Description |
|---------|------|-------------|
| `cpu` | 8000 | CPU-only processing |
| `gpu` | 8001 | GPU-accelerated processing |
| `dev` | 8002 | Development mode with code mounting |

### Resource Requirements

**CPU Mode:**
- Memory: 8GB recommended
- CPU: 4+ cores recommended
- Storage: 5GB for container + models

**GPU Mode:**
- Memory: 12GB recommended
- GPU: NVIDIA GPU with 8GB+ VRAM
- CUDA: Compatible NVIDIA Docker runtime

## Integration with LucidKaraoke

The C++ application automatically manages the Docker container:

1. **HttpStemProcessor**: New processor class that communicates with the Docker service
2. **DockerManager**: Utility for container lifecycle management
3. **Automatic Startup**: Container is started automatically when needed
4. **Health Monitoring**: Service health is checked before processing

### Usage in C++

```cpp
#include "HttpStemProcessor.h"

// Create processor
HttpStemProcessor processor(inputFile, outputDirectory);

// Configure (optional)
processor.setContainerPort(8000);
processor.setAutoStartContainer(true);

// Set callbacks
processor.onProgressUpdate = [](double progress, const juce::String& message) {
    std::cout << "Progress: " << progress << " - " << message << std::endl;
};

processor.onProcessingComplete = [](bool success, const juce::String& message) {
    std::cout << "Complete: " << (success ? "Success" : "Failed") << " - " << message << std::endl;
};

// Start processing
processor.startThread();
```

## Troubleshooting

### Container Won't Start

1. Check Docker is running: `docker info`
2. Check for port conflicts: `lsof -i :8000`
3. Check container logs: `docker compose logs`

### Service Not Responding

1. Wait for initialization (first start takes longer)
2. Check health endpoint: `curl http://localhost:8000/health`
3. Check available resources (memory/GPU)

### Processing Failures

1. Verify audio file format is supported
2. Check file size (large files may timeout)
3. Monitor container logs: `docker compose logs -f`

### GPU Issues

1. Verify NVIDIA Docker runtime: `docker run --rm --gpus all nvidia/cuda:11.0-base nvidia-smi`
2. Check CUDA compatibility
3. Fall back to CPU mode if needed

## Development

### Local Development

For development with code changes:

```bash
# Start in development mode
docker compose --profile dev up -d

# Code changes are automatically reloaded
# Logs are available via:
docker compose logs -f demucs-dev
```

### Building Custom Images

```bash
# Build with specific base image
docker build --build-arg BASE_IMAGE=pytorch/pytorch:2.1.0-cuda12.1-cudnn8-runtime .

# Build CPU-only image
docker build --build-arg CUDA_SUPPORT=false .
```

### Extending the Service

The FastAPI service can be extended with additional endpoints:

1. Edit `app/main.py` for new endpoints
2. Modify `app/stem_processor.py` for processing logic
3. Update `requirements.txt` for new dependencies
4. Rebuild the container

## Cloud Deployment

### Google Cloud Run

The container is designed to be compatible with Google Cloud Run:

```bash
# Build for Cloud Run
docker build -t gcr.io/your-project/demucs-service .

# Push to Google Container Registry
docker push gcr.io/your-project/demucs-service

# Deploy to Cloud Run
gcloud run deploy demucs-service \
  --image gcr.io/your-project/demucs-service \
  --platform managed \
  --memory 8Gi \
  --cpu 4 \
  --timeout 600
```

### Other Platforms

The service is compatible with:
- AWS Lambda (with container support)
- Azure Container Instances
- Kubernetes
- Any Docker-compatible platform

## Security

- Service runs as non-root user
- Input validation for uploaded files
- Temporary file cleanup
- No persistent storage of user data
- CORS configured for web access