#!/bin/bash
# Script kiểm tra tài nguyên hệ thống theo thời gian thực

echo "📊 Đang theo dõi tài nguyên Pi 4... (Nhấn Ctrl+C để dừng)"
echo "---------------------------------------------------------"

while true; do
    # Nhiệt độ
    TEMP=$(vcgencmd measure_temp | cut -d'=' -f2)
    
    # % CPU (Lấy trung bình tải)
    CPU_LOAD=$(top -bn1 | grep "Cpu(s)" | sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk '{print 100 - $1"%"}')
    
    # RAM
    MEM_USED=$(free -m | awk '/Mem:/ { printf("%d/%d MB (%.1f%%)", $3, $2, $3/$2*100) }')
    
    echo -ne "\r🌡️ Nhiệt độ: $TEMP | 🧠 CPU: $CPU_LOAD | 💾 RAM: $MEM_USED    "
    sleep 1
done
