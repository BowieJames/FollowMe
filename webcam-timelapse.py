import cv2
import time
import os
output_dir = "captured_images"
os.makedirs(output_dir, exist_ok=True)
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("ERROR no webcam.")
    exit()

cap.set(cv2.CAP_PROP_FRAME_WIDTH,320)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT,240)

actual_width = cap.get(cv2.CAP_PROP_FRAME_WIDTH)
actual_height = cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
print(f"Resolution set to: {int(actual_width)}x{int(actual_height)}")

photo_count = 0
start_time = time.time()
duration = 60 #seconds

while time.time() - start_time < duration:
    ret, frame = cap.read()
    if not ret:
        print("ERROR failed to capture image")
        break

    filename = os.path.join(output_dir, f"img_{photo_count:04d}.jpg")
    cv2.imwrite(filename, frame)
    print(f"SAVED {filename}")
    photo_count += 1
    time.sleep(1) #delay between shots

cap.release()
cv2.destroyAllWindows()
print("Capture completed")
