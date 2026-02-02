import cv2
import sys

def test_camera(index):
    print(f"Testing Camera Index: {index}...")
    cap = cv2.VideoCapture(index, cv2.CAP_V4L2)
    
    if not cap.isOpened():
        print(f"❌ Failed to open camera index {index}")
        return False
    
    # Try to grab a frame
    ret, frame = cap.read()
    if ret:
        filename = f"test_cam_{index}.jpg"
        cv2.imwrite(filename, frame)
        print(f"✅ Success! Captured frame saved as {filename}")
        print(f"   Resolution: {frame.shape[1]}x{frame.shape[0]}")
    else:
        print(f"❌ Opened camera {index} but could not read frame (empty frame)")
    
    cap.release()
    return ret

if __name__ == "__main__":
    # Test common indices
    for i in range(4):
        test_camera(i)
        print("-" * 30)
