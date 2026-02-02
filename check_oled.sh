#!/bin/bash
# Script kiểm tra và dò tìm màn hình OLED trên Raspberry Pi 4

echo "🔍 --- KIỂM TRA KẾT NỐI OLED ---"

# 1. Kiểm tra driver I2C trong kernel
echo -n "1. Kiểm tra Driver I2C: "
if lsmod | grep -q i2c_dev; then
    echo "✅ Đã load (i2c_dev)"
else
    echo "❌ Chưa load. Đang thử load tạm thời..."
    sudo modprobe i2c-dev
fi

# 2. Kiểm tra các file thiết bị I2C
echo -e "\n2. Các bus I2C hiện có trong /dev/:"
ls -l /dev/i2c*

# 3. Dò tìm địa chỉ I2C (Scan)
echo -e "\n3. Đang quét bus I2C-1 (Mặc định)..."
if command -v i2cdetect &> /dev/null; then
    SCAN_RESULT=$(sudo i2cdetect -y 1)
    echo "$SCAN_RESULT"
    
    if echo "$SCAN_RESULT" | grep -q "3c"; then
        echo -e "\n🎉 THÀNH CÔNG: Tìm thấy màn hình OLED tại địa chỉ 0x3C!"
    else
        echo -e "\n⚠️ CẢNH BÁO: Không tìm thấy thiết bị nào tại địa chỉ 0x3C trên bus 1."
        echo "Đang thử quét bus I2C-0 (Dành cho các dòng Pi cũ hoặc cấu hình đặc biệt)..."
        sudo i2cdetect -y 0 2>/dev/null || echo "Bus 0 không khả dụng."
    fi
else
    echo "❌ Lỗi: i2c-tools chưa được cài đặt. Hãy chạy ./setup.sh"
fi

# 4. Kiểm tra quyền truy cập
echo -e "\n4. Kiểm tra quyền truy cập /dev/i2c-1:"
ls -l /dev/i2c-1

# 5. Kiểm tra xem có tiến trình nào đang chiếm I2C không
echo -e "\n5. Kiểm tra tiến trình sử dụng I2C (nếu có):"
if command -v fuser &> /dev/null; then
    sudo fuser -v /dev/i2c-1 2>/dev/null || echo "Không có tiến trình nào đang chiếm giữ I2C."
fi

echo -e "\n--- KẾT THÚC KIỂM TRA ---"
echo "Mẹo: Nếu không thấy 3c, hãy kiểm tra lại dây SCL/SDA và đảm bảo chân VCC đã cấp nguồn 3.3V/5V."
