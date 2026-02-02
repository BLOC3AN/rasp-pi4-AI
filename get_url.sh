#!/bin/bash
# Helper script to get the VLC/Browser streaming URL
IP=$(hostname -I | awk '{print $1}')
PORT=$(grep WEBSERVER_PORT .env | cut -d'=' -f2 | xargs)
echo "------------------------------------------------"
echo "📷 Raspberry Pi OCR Streaming URL:"
echo -e "\033[32mhttp://$IP:$PORT\033[0m"
echo -e "\033[36mhttp://$IP:$PORT/stream\033[0m (Direct MJPEG for VLC)"
echo "------------------------------------------------"
