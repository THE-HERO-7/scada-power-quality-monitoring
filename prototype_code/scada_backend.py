import sys
import csv
import numpy as np
import pandas as pd
from scipy.fft import fft
from sklearn.ensemble import IsolationForest
from sklearn.preprocessing import StandardScaler

# -------- CONFIG --------
DATASET_FILE = "dataset.csv"
WINDOW = 200 

# -------- ML GLOBALS --------
model = IsolationForest(contamination=0.05, random_state=42)
scaler = StandardScaler()
is_trained = False

def pre_train():
    global is_trained, model, scaler
    try:
        # Load the CSV
        df = pd.read_csv(DATASET_FILE)
        
        # This line ensures we only take numeric rows 
        # (in case there are extra headers inside the file)
        df = df.apply(pd.to_numeric, errors='coerce').dropna()
        
        # Use the first 5 columns for training
        X = df.iloc[:, :5].values 
        
        if len(X) > 10:
            scaler.fit(X)
            X_scaled = scaler.transform(X)
            model.fit(X_scaled)
            is_trained = True
            print("INFO: Model pre-trained successfully.", file=sys.stderr)
        else:
            print("ERROR: CSV has no numeric data yet.", file=sys.stderr)
    except Exception as e:
        print(f"ERROR: Pre-train failed: {e}", file=sys.stderr)


def calculate_thd(signal):
    if len(signal) == 0: return 0
    fft_vals = np.abs(fft(signal))
    fundamental = np.max(fft_vals[1:int(len(fft_vals)/2)])
    total_sum_sq = np.sum(fft_vals[1:int(len(fft_vals)/2)]**2)
    harmonics_sum_sq = total_sum_sq - (fundamental**2)
    if fundamental <= 0: return 0
    return np.sqrt(max(0, harmonics_sum_sq)) / fundamental

# Initial Training
pre_train()

wave_buffer = []
baseline_thd = None

# -------- MAIN LOOP --------
for line in sys.stdin:
    line = line.strip()
    if not line: continue
    parts = line.split(",")

    if parts[0] == "W":
        try:
            wave_buffer = np.array(list(map(float, parts[1:])))
        except: continue

    elif parts[0] == "F":
        try:
            features = [float(parts[1]), float(parts[2]), float(parts[3]), float(parts[4])]
            if len(wave_buffer) == 0: continue
            
            thd = calculate_thd(wave_buffer)
            features.append(thd) # Vector: [RMS, Peak, Freq, Crest, THD]

            status = "NORMAL"
            score = 0.0  # Default score

            if is_trained:
                X_input = scaler.transform([features])
                # predict() returns 1 (normal) or -1 (anomaly)
                pred = model.predict(X_input)[0]
                # decision_function() returns the raw anomaly score
                score = model.decision_function(X_input)[0]
                
                if pred == -1:
                    status = "ANOMALY"

            # -------- OUTPUT TO ELECTRON --------
            # Format: STATUS, [STATUS_TEXT], [THD], [SCORE]
            print(f"STATUS,{status},{thd:.4f},{score:.4f}")
            sys.stdout.flush()

        except Exception as e:
            print(f"Prediction Error: {e}", file=sys.stderr)
