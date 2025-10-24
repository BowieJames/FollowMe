import serial
import time
import matplotlib.pyplot as plt
from collections import deque
import threading

# === Configuration ===
PORTS = ['/dev/ttyUSB0', '/dev/ttyUSB1', '/dev/ttyUSB2']  # add or remove as needed
BAUD = 115200
WINDOW = 100

# === Create a serial connection for each device ===
serial_ports = []
for p in PORTS:
    try:
        s = serial.Serial(p, BAUD, timeout=1)
        serial_ports.append(s)
        print(f"Connected to {p}")
        time.sleep(1)
    except serial.SerialException:
        print(f"Failed to open {p}")

# === Prepare plot ===
plt.ion()
fig, ax = plt.subplots()
colors = ['b', 'r', 'g']  # colors for each serial device
ydata_list = [deque([0.0]*WINDOW, maxlen=WINDOW) for _ in serial_ports]
xdata = list(range(WINDOW))
lines = [ax.plot(xdata, ydata, color)[0] for ydata, color in zip(ydata_list, colors)]

ax.set_ylim(0, 5)
ax.set_xlabel('Samples')
ax.set_ylabel('Value')
ax.set_title('Live Serial Data from Multiple Devices')
ax.legend(PORTS)

# === Shared data lock for thread safety ===
lock = threading.Lock()

# === Function to read from each serial port ===
def read_serial(port_index, ser):
    ignore_words = ['7D', 'error']
    while True:
        try:
            line_raw = ser.readline().decode('utf-8', errors='ignore').strip()
            if not line_raw:
                continue
            if any(word in line_raw for word in ignore_words):
                continue
            try:
                value = float(line_raw)
                with lock:
                    ydata_list[port_index].append(value)
            except ValueError:
                continue
        except serial.SerialException:
            print(f"Serial error on {PORTS[port_index]}")
            break

# === Start a thread for each serial device ===
threads = []
for i, ser in enumerate(serial_ports):
    t = threading.Thread(target=read_serial, args=(i, ser), daemon=True)
    t.start()
    threads.append(t)

# === Plot update loop ===
try:
    while True:
        with lock:
            for i, line in enumerate(lines):
                line.set_ydata(ydata_list[i])
            ax.relim()
            ax.autoscale_view(True, True, True)
        plt.pause(0.05)
except KeyboardInterrupt:
    print("\nStopping...")
finally:
    for s in serial_ports:
        s.close()
    print("All serial ports closed.")
