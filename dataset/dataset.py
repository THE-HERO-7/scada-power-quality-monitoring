import serial
import csv

PORT = 'COM8'
BAUD = 115200
TARGET_SAMPLES = 100

ser = serial.Serial(PORT, BAUD, timeout=1)

with open('normal_data.csv', 'w', newline='') as f:
    writer = csv.writer(f)
    
    writer.writerow(["RMS", "Peak", "Freq", "Crest"])

    print("Collecting NORMAL data (charger OFF)...")

    count = 0
    while count < TARGET_SAMPLES:
        line = ser.readline().decode(errors='ignore').strip()
        parts = line.split(',')

        if len(parts) != 5:
            continue

        try:
            rms = float(parts[0])
            peak = float(parts[1])
            freq = float(parts[2])
            crest = float(parts[3])
            label = int(parts[4])
        except:
            continue

        # Only store NORMAL data
        if label == 0:
            writer.writerow([rms, peak, freq, crest])
            count += 1
            print(f"Saved {count}")

print("Dataset ready!")
