#!/bin/bash
# Cài đặt trình biên dịch và các thư viện phát triển cho Raspberry Pi 4 OCR
echo "📦 Installing C++ dependencies for OCR project..."
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config \
    libopencv-dev \
    libtesseract-dev \
    libleptonica-dev \
    libi2c-dev \
    i2c-tools \
    v4l-utils \
    tesseract-ocr-vie \
    libcurl4-openssl-dev

echo "✅ Dependencies installed."
