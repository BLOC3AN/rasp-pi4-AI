#!/bin/bash
# Script kiểm tra phần cứng camera trên Raspberry Pi 4

echo "🔍 --- KIỂM TRA PHẦN CỨNG CAMERA ---"

# 1. Kiểm tra driver qua vcgencmd (Cho Legacy stack)
echo -n "1. Trạng thái vcgencmd: "
vcgencmd get_camera

# 2. Liệt kê các thiết bị video hiện có
echo -e "\n2. Danh sách thiết bị /dev/video*:"
ls -l /dev/video*

# 3. Chi tiết thiết bị qua v4l2-ctl
if command -v v4l2-ctl &> /dev/null; then
    echo -e "\n3. Chi tiết từ v4l2-ctl:"
    v4l2-ctl --list-devices
else
    echo -e "\n3. (v4l2-ctl chưa được cài đặt, hãy chạy ./setup.sh)"
fi

# 4. Kiểm tra xem có tiến trình nào đang chiếm camera không
echo -e "\n4. Kiểm tra tiến trình đang sử dụng /dev/video0:"
if command -v fuser &> /dev/null; then
    sudo fuser -v /dev/video0
else
    echo "fuser not found"
fi

echo -e "\n--- KẾT THÚC KIỂM TRA ---"
