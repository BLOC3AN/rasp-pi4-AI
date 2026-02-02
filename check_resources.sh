#!/bin/bash
# Script kiểm tra tài nguyên hệ thống theo thời gian thực

echo "📊 Đang theo dõi tài nguyên Pi 4... (Nhấn Ctrl+C để dừng)"
echo "---------------------------------------------------------"

while true; do
    # Nhiệt độ & Xung nhịp
    TEMP=$(vcgencmd measure_temp | cut -d'=' -f2)
    FREQ=$(vcgencmd measure_clock arm | awk -F= '{printf "%.2f GHz", $2/1000000000}')
    
    # % CPU
    CPU_USAGE=$(top -bn1 | grep "Cpu(s)" | sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk '{print 100 - $1"%"}')
    
    # RAM
    MEM_USED=$(free -m | awk '/Mem:/ { printf("%d/%d MB (%.1f%%)", $3, $2, $3/$2*100) }')
    
    # OCR App Status
    if pgrep -x "ocr_app" > /dev/null; then
        OCR_STATS=$(top -bn1 -p $(pgrep -px ocr_app) | tail -n 1 | awk '{print "CPU: "$9"% | MEM: "$10"%"}')
        OCR_STATUS="\033[32mRUNNING\033[0m ($OCR_STATS)"
    else
        OCR_STATUS="\033[31mSTOPPED\033[0m"
    fi

    echo -ne "\r🌡️ $TEMP | 🧠 CPU: $CPU_USAGE ($FREQ) | 💾 RAM: $MEM_USED | 🎯 APP: $OCR_STATUS    "
    sleep 1
done
