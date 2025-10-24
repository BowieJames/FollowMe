import serial
import time
import re
import matplotlib.pyplot as plt
from collections import deque

# === Configuration ===
PORT = '/dev/ttyUSB0'   # Change to your serial device (e.g., /dev/ttyUSB1)
BAUD = 115200           # Match your device's baud rate
WINDOW = 100            # Number of points to show in the plot

# === Setup serial connection ===
ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)  # Allow time for device to reset

# === Prepare live plot ===
plt.ion()
fig, ax = plt.subplots()
ydata = deque([0.0]*WINDOW, maxlen=WINDOW)
xdata = list(range(WINDOW))
(line,) = ax.plot(xdata, ydata, '-b')
ax.set_ylim(0, 5)   # Adjust to your expected value range
ax.set_xlabel('Samples')
ax.set_ylabel('Value')
ax.set_title('Live Serial Data Stream')

print(f"Reading from {PORT} (Ctrl+C to stop)\n")

# === Main read and plot loop ===
try:
    while True:
        ignore_words = ['7D', 'error']
        line_raw = ser.readline().decode('utf-8', errors='ignore').strip()
        if any(word in line_raw for word in ignore_words):
            print("Ignored:", line_raw)
        else:
            try:
                value = float(line_raw)
                ydata.append(value)
                line.set_ydata(ydata)
                ax.relim()
                ax.autoscale_view(True, True, True)
                plt.pause(0.01)
                # use value here
            except ValueError:
                print("Invalid numeric input:", line_raw)

except KeyboardInterrupt:
    print("\nStopping plot...")

finally:
    ser.close()
    print("Serial port closed.")
