#include <Arduino.h>

#define ADC_PIN A0
#define SAMPLE_COUNT 200
#define SAMPLE_DELAY_US 200   // 5 kHz sampling

#define VREF 3.3
#define ADC_RES 4095.0

float samples[SAMPLE_COUNT];

// ---------- RMS ----------
float calculateRMS(float *data) {
  float sum = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    sum += data[i] * data[i];
  }
  return sqrt(sum / SAMPLE_COUNT);
}

// ---------- Peak ----------
float calculatePeak(float *data) {
  float peak = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    float val = abs(data[i]);
    if (val > peak) peak = val;
  }
  return peak;
}

// ---------- Frequency ----------
float calculateFrequency(float *data) {
  int crossings = 0;

  for (int i = 1; i < SAMPLE_COUNT; i++) {
    if ((data[i - 1] < 0 && data[i] >= 0) ||
        (data[i - 1] > 0 && data[i] <= 0)) {
      crossings++;
    }
  }

  float duration = (SAMPLE_COUNT * SAMPLE_DELAY_US) / 1000000.0;

  if (duration == 0) return 0;

  return (crossings / 2.0) / duration;
}

void loop() {

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    int raw = analogRead(ADC_PIN);

    // Convert to voltage (0–3.3V)
    float voltage = (raw / ADC_RES) * VREF;

    samples[i] = voltage;

    delayMicroseconds(SAMPLE_DELAY_US);
  }

  float mean = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    mean += samples[i];
  }
  mean /= SAMPLE_COUNT;

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    samples[i] -= mean;   // AC centered waveform
  }

  // ---------- 3. FEATURES ----------
  float rms = calculateRMS(samples);
  float peak = calculatePeak(samples);
  float freq = calculateFrequency(samples);
  float crest = (rms != 0) ? peak / rms : 0;

  // ---------- 4. SEND WAVEFORM ----------
  Serial.print("W,");

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    Serial.print(samples[i], 4);

    if (i < SAMPLE_COUNT - 1) Serial.print(",");
  }
  Serial.println();

  // ---------- 5. SEND FEATURES ----------
  Serial.print("F,");
  Serial.print(rms, 4);
  Serial.print(",");
  Serial.print(peak, 4);
  Serial.print(",");
  Serial.print(freq, 2);
  Serial.print(",");
  Serial.println(crest, 3);

  delay(20);  // stability
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
}
