# app.py
# SCADA Frontend Dashboard (HMI) using Streamlit

import streamlit as st
import serial
import csv
from datetime import datetime
import numpy as np
import time

# ---------------- CONFIG ----------------
SERIAL_PORT = 'COM13'      # change this
BAUD_RATE = 115200
SAMPLE_COUNT = 400
CSV_FILE = 'scada_log.csv'
# ----------------------------------------

st.set_page_config(page_title="Embedded SCADA Dashboard", layout="wide")

st.title("⚡ Embedded Signal Monitoring – SCADA Dashboard")

# Sidebar controls
st.sidebar.header("Controls")
start_btn = st.sidebar.button("▶ Start Monitoring")
stop_btn = st.sidebar.button("⏹ Stop Monitoring")

status_box = st.sidebar.empty()

# Placeholders
chart_placeholder = st.empty()
metrics_col1, metrics_col2, metrics_col3 = st.columns(3)

rms_box = metrics_col1.metric("RMS (ADC units)", "--")
freq_box = metrics_col2.metric("Frequency (Hz)", "--")
status_metric = metrics_col3.metric("System Status", "IDLE")

# Internal state
if "running" not in st.session_state:
    st.session_state.running = False

if start_btn:
    st.session_state.running = True

if stop_btn:
    st.session_state.running = False

# Open serial lazily
ser = None
csv_file = None
csv_writer = None

if st.session_state.running:
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        status_metric.metric("System Status", "RUNNING")
        status_box.success("Connected to device")

        csv_file = open(CSV_FILE, 'a', newline='')
        csv_writer = csv.writer(csv_file)

        if csv_file.tell() == 0:
            header = ['timestamp'] + [f's{i}' for i in range(SAMPLE_COUNT)]
            csv_writer.writerow(header)

        while st.session_state.running:
            line_raw = ser.readline().decode().strip()
            if not line_raw:
                continue

            parts = line_raw.split(',')
            if len(parts) != SAMPLE_COUNT:
                continue

            samples = np.array(list(map(int, parts)))

            # Log
            timestamp = datetime.now().isoformat()
            csv_writer.writerow([timestamp] + samples.tolist())
            csv_file.flush()

            # RMS calculation
            rms_val = np.sqrt(np.mean(samples**2))

            # Display
            chart_placeholder.line_chart(samples)
            rms_box.metric("RMS (ADC units)", f"{rms_val:.2f}")
            freq_box.metric("Frequency (Hz)", "--")

            time.sleep(0.05)

    except Exception as e:
        status_metric.metric("System Status", "ERROR")
        status_box.error(str(e))

    finally:
        if ser:
            ser.close()
        if csv_file:
            csv_file.close()

else:
    status_metric.metric("System Status", "IDLE")
    status_box.info("System stopped")
