# phase_c_scada.py
# Live waveform plot + CSV logger for Arduino ADC frames

import serial
import time
import csv
from datetime import datetime
import matplotlib.pyplot as plt

# ================= CONFIG =================
SERIAL_PORT = 'COM13'      # change this (e.g., /dev/ttyUSB0 on Linux)
BAUD_RATE = 115200
SAMPLE_COUNT = 200
CSV_FILE = 'scada_log.csv'
# =========================================

# Open serial
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
print("Connected to", SERIAL_PORT)

# Open CSV file
csv_file = open(CSV_FILE, 'a', newline='')
csv_writer = csv.writer(csv_file)

# Write header if file is empty
if csv_file.tell() == 0:
    header = ['timestamp'] + [f's{i}' for i in range(SAMPLE_COUNT)]
    csv_writer.writerow(header)

# Setup plot
plt.ion()
fig, ax = plt.subplots()
fig.set_size_inches(20, 20)
x = list(range(SAMPLE_COUNT))
y = [0]*SAMPLE_COUNT
line, = ax.plot(x, y)
ax.set_title("Live ADC Waveform")
ax.set_xlabel("Sample Index")
ax.set_ylabel("ADC Value")
ax.set_ylim(0, 4095)

print("SCADA backend running... Press Ctrl+C to stop.")

try:
    while True:
        line_raw = ser.readline().decode().strip()

        if not line_raw:
            continue

        parts = line_raw.split(',')
        if len(parts) != SAMPLE_COUNT:
            continue

        samples = list(map(int, parts))

        # Log to CSV
        timestamp = datetime.now().isoformat()
        csv_writer.writerow([timestamp] + samples)
        csv_file.flush()

        # Update plot
        line.set_ydata(samples)
        ax.draw_artist(ax.patch)
        ax.draw_artist(line)
        fig.canvas.flush_events()

except KeyboardInterrupt:
    print("Stopping...")

finally:
    ser.close()
    csv_file.close()
    plt.ioff()
    plt.show()
