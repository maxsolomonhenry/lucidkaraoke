#!/bin/bash

# Test script for DeMucs Docker service
set -e

SERVICE_URL="${SERVICE_URL:-http://localhost:8000}"

echo "Testing DeMucs Docker service at $SERVICE_URL"
echo "=============================================="

# Test 1: Health check
echo "1. Testing health endpoint..."
if curl -s -f "$SERVICE_URL/health" > /dev/null; then
    echo "✓ Health check passed"
    curl -s "$SERVICE_URL/health" | python3 -m json.tool 2>/dev/null || echo "Service is responding"
else
    echo "✗ Health check failed"
    echo "Make sure the service is running:"
    echo "  docker compose --profile cpu up -d"
    exit 1
fi

echo ""

# Test 2: Models endpoint
echo "2. Testing models endpoint..."
if curl -s -f "$SERVICE_URL/models" > /dev/null; then
    echo "✓ Models endpoint passed"
    curl -s "$SERVICE_URL/models" | python3 -m json.tool 2>/dev/null || echo "Models endpoint is responding"
else
    echo "✗ Models endpoint failed"
fi

echo ""

# Test 3: Root endpoint
echo "3. Testing root endpoint..."
if curl -s -f "$SERVICE_URL/" > /dev/null; then
    echo "✓ Root endpoint passed"
else
    echo "✗ Root endpoint failed"
fi

echo ""

# Test 4: Stem separation (if test file provided)
if [ -n "$TEST_AUDIO_FILE" ] && [ -f "$TEST_AUDIO_FILE" ]; then
    echo "4. Testing stem separation with: $TEST_AUDIO_FILE"
    
    OUTPUT_FILE="test_stems_$(date +%s).zip"
    
    if curl -X POST \
         -F "audio_file=@$TEST_AUDIO_FILE" \
         -F "format=mp3" \
         -F "bitrate=320" \
         "$SERVICE_URL/separate" \
         --output "$OUTPUT_FILE" \
         --progress-bar; then
        
        if [ -f "$OUTPUT_FILE" ] && [ -s "$OUTPUT_FILE" ]; then
            echo "✓ Stem separation completed successfully"
            echo "  Output file: $OUTPUT_FILE ($(du -h "$OUTPUT_FILE" | cut -f1))"
            
            # Try to list contents of ZIP
            if command -v unzip &> /dev/null; then
                echo "  ZIP contents:"
                unzip -l "$OUTPUT_FILE" | grep -E '\.(mp3|wav|flac)$' || echo "    (Could not list contents)"
            fi
        else
            echo "✗ Stem separation failed - no output file generated"
        fi
    else
        echo "✗ Stem separation request failed"
    fi
else
    echo "4. Skipping stem separation test (no test file provided)"
    echo "   To test with an audio file, run:"
    echo "   TEST_AUDIO_FILE=/path/to/audio.mp3 $0"
fi

echo ""
echo "Test completed!"
echo ""
echo "Service endpoints:"
echo "  Health:    $SERVICE_URL/health"
echo "  Models:    $SERVICE_URL/models"
echo "  Separate:  POST $SERVICE_URL/separate"