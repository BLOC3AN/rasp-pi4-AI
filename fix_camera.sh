#!/bin/bash
# Script giải phóng camera bị chiếm dụng

VIDEO_DEV="/dev/video0"

echo "🛑 Đang kiểm tra các tiến trình chiếm dụng $VIDEO_DEV..."

# Tìm các PID
PIDS=$(sudo fuser $VIDEO_DEV 2>/dev/null)

if [ -z "$PIDS" ]; then
    echo "✅ Không có tiến trình nào đang sử dụng Camera."
else
    echo "⚠️ Phát hiện các PID đang chạy: $PIDS"
    for PID in $PIDS; do
        PROC_NAME=$(ps -p $PID -o comm=)
        echo "🔥 Đang buộc dừng tiến trình: $PID ($PROC_NAME)..."
        sudo kill -9 $PID
    done
    echo "✅ Đã giải phóng Camera thành công!"
fi
