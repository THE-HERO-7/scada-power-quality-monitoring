# Real-Time Embedded Signal Monitoring & SCADA System

A complete embedded data acquisition and SCADA-style monitoring system built using Arduino and Python for real-time waveform visualization, logging, and analysis of analog electrical signals.

This project mimics an industrial architecture with a **Remote Terminal Unit (RTU)** for signal acquisition and a **supervisory layer (SCADA/HMI)** for monitoring and control.

---

## Features (Current)

### Embedded / Hardware

* 12-bit ADC sampling using Arduino Nano 33 BLE Sense Rev2
* Custom analog front-end for AC signals

  * Mid-supply biasing (≈1.65V)
  * Voltage scaling using resistor divider
  * RC low-pass filtering (anti-aliasing + noise suppression)
  * ADC input protection resistor
* CSV frame-based serial telemetry (RTU behavior)

### Software / System

* Python backend for:

  * Serial data parsing
  * Frame validation
  * Timestamped CSV logging
  * Real-time processing pipeline
* Streamlit-based SCADA dashboard (HMI):

  * Live waveform visualization
  * RMS computation
  * System status indicators (IDLE / RUNNING / ERROR)
  * Start / Stop monitoring controls

---

## Architecture

```
Signal Source
     ↓
Analog Signal Conditioning (bias + scaling + filter)
     ↓
Arduino ADC (RTU)
     ↓
Serial Telemetry (UART)
     ↓
Python Backend (processing + logging)
     ↓
SCADA Dashboard (Streamlit HMI)
```

---

## Repository Structure

```
prototype_code/
  ├── arduino_adc_stream.ino      # Embedded firmware (ADC + telemetry)
  ├── scada_backend.py            # Python backend (logger + parser)
  └── scada_frontend.py           # Streamlit SCADA dashboard (HMI)

prototype_circut/
  └── prototype_photo.jpg         # Hardware prototype image
```

---

## Hardware Prototype

Current breadboard implementation of the ADC front-end and biasing circuit:

![Hardware Prototype](prototype_circut/prototype_photo.jpg)

---

## Current Status

| Module                             | Status         |
| ---------------------------------- | -------------- |
| ADC acquisition pipeline           | ✅ Completed    |
| Biasing & signal conditioning      | ✅ Completed    |
| Serial telemetry (RTU)             | ✅ Completed    |
| Python backend (logging + parsing) | ✅ Completed    |
| SCADA frontend (HMI)               | ✅ Completed    |
| Transformer-isolated AC sensing    | 🟡 In Progress |
| Mains voltage monitoring           | 🟡 Planned     |
| Frequency estimation               | 🔜 Planned     |
| FFT / harmonic analysis            | 🔜 Planned     |
| ML-based anomaly detection         | 🔜 Planned     |

---

## Technologies Used

* Arduino (Embedded C++)
* Python
* Streamlit
* Serial Communication (UART)
* Analog Signal Conditioning
* ADC Sampling
* Real-time Data Visualization
* CSV Data Logging

---

## Validation

* Synthetic sine wave testing using PWM + RC filtering
* Bias stability verification
* Serial data integrity testing
* Real-time visualization through SCADA dashboard
* Low-voltage AC testing using transformer isolation (ongoing)

---

## Roadmap (Next Steps)

### Signal Analysis

* Frequency estimation (zero-crossing / FFT-based)
* RMS voltage calibration to real-world units
* Peak and crest factor detection

### Power Quality

* FFT spectrum visualization
* Harmonic distortion (THD) computation
* Voltage sag/swell detection

### System

* Alarm thresholds & event logging
* Historical trend plots
* Exportable reports

### AI / ML (Optional)

* Feature extraction from waveform data
* Unsupervised clustering of abnormal events
* Anomaly detection on long-term logs

### Hardware

* Dedicated AC voltage sensing module (e.g., ZMPT101B)
* PCB design for analog front-end
* Improved isolation and calibration

---

## Notes

This project is actively being developed and extended. The current implementation focuses on building a reliable acquisition and monitoring pipeline, with advanced signal analysis and full mains integration planned as future upgrades.

---

## Author

AJ

---

> This project is intended for educational and research purposes. Direct mains voltage experimentation must be performed only with proper isolation and safety precautions.
