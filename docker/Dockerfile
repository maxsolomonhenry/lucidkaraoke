# Minimal CUDA runtime for DeMucs CLI service
# Much smaller than full PyTorch images

FROM nvidia/cuda:11.8.0-cudnn8-runtime-ubuntu22.04

# Set environment variables
ENV PYTHONUNBUFFERED=1
ENV TORCH_HOME=/app/models
ENV DEBIAN_FRONTEND=noninteractive

# Install Python and system dependencies
RUN apt-get update && apt-get install -y \
    python3.10 \
    python3-pip \
    python3.10-dev \
    ffmpeg \
    curl \
    && rm -rf /var/lib/apt/lists/* \
    && ln -s /usr/bin/python3.10 /usr/bin/python
    

# Create non-root user
RUN useradd -m -u 1000 demucs && \
    mkdir -p /app /app/models /app/temp && \
    chown -R demucs:demucs /app

# Set working directory
WORKDIR /app

# Copy requirements and install Python dependencies
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Pre-download DeMucs model weights to avoid cold start downloads
RUN python -c "from demucs.pretrained import get_model; get_model('htdemucs_ft')" && \
    chown -R demucs:demucs /app/models

# Switch to non-root user
USER demucs

# Copy application code
COPY --chown=demucs:demucs app/ ./

# Expose port
EXPOSE 8000

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8000/health || exit 1

# Default command
CMD ["python", "main.py"]