#include <Arduino.h>
#define ADC_PIN A0
#define SAMPLE_COUNT 400
#define SAMPLE_DELAY_US 200   // 5 kHz

int samples[SAMPLE_COUNT];

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
}

void loop() {
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    samples[i] = analogRead(ADC_PIN);
    delayMicroseconds(SAMPLE_DELAY_US);
    
  }

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    Serial.print(samples[i]);
    if (i < SAMPLE_COUNT - 1) Serial.print(",");
  }
  Serial.println();
}
