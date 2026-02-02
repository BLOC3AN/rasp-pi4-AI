#!/bin/bash
# Script để biên dịch dự án OCR trên Pi 4
mkdir -p build
cd build
echo "🔨 Running CMake..."
cmake ..
echo "🚀 Compiling code..."
make -j$(nproc)
echo "✅ Build finished. Run with: sudo ./ocr_app"
