import serial
import time

# Configure the serial port
ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)

try:
    while True:
        for angle in range(361):  # 0 to 360 inclusive
            ser.write(f"{angle}\n".encode())  # Send the number as text with newline
            print(f"Sent: {angle}")
            time.sleep(0.5)  # 50 ms delay between numbers (adjust as needed)

except KeyboardInterrupt:
    print("\nStopped by user.")

finally:
    ser.close()
    print("Serial connection closed.")