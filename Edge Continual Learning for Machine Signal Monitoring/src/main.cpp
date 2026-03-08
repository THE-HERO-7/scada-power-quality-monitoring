#include <Arduino.h>
#include <Arduino_BMI270_BMM150.h>
#include <arduinoFFT.h> 
#include "classifier_1.h" // Your XGBoost Model
#include "svm_1.h"        // Your SVM Model (Gatekeeper)

/**A
 * ARCHITECTURE:
 * 1. Sensing: BMI270 Accelerometer (Signal Magnitude Vector)
 * 2. Feature Extraction: 11 Features (Time + Frequency Domain)
 * 3. Logic:
 * - If SVM identifies signal as "Normal/Known" -> Run XGBoost Classification.
 * - If SVM identifies signal as "Anomaly/Unknown" -> Send RAW features to Cloud (PC).
 */

// ===== CONFIGURATION =====
#define WINDOW 256
#define SAMPLING_FREQ 100.0 // Hz
float buffer[WINDOW];
int bufferIdx = 0;
int signalSource = 0; // 0 for IMU

// FFT Objects
arduinoFFT FFT = arduinoFFT();
double vReal[WINDOW];
double vImag[WINDOW];

// ===== MODELS =====
Eloquent::ML::Port::XGBClassifier xgb;
Eloquent::ML::Port::one_class_svm svm;

// ===== BASELINE SCALER (CWRU) =====
// These will be updated by the Cloud during CIL retraining
// float mean[11] = {
//     0.3502076, 0.2168899, 1.696139, 3.3294788,
//     -0.0358876, 6.543861, 672.6005, 2.196885,
//     0.0045953, 0.0000388, 0.000000193
// };

// float stdv[11] = {
//     0.3073576, 0.3458983, 1.6461289, 4.959931,
//     0.1901155, 3.147635, 193.0330, 0.4283016,
//     0.0073736, 0.0000571, 0.000000160
// };
float mean[11] = {0.36548249, 0.05965945, 1.14912136, 1.66495588, 1.52272923, 4.90231832, 436.6772874, 2.88885578, 0.13919788, 0.04613171, 0.0203375};
float stdv[11] = {0.38117382, 0.09815811, 0.99485802, 2.27128853, 3.77948293, 2.15400952, 320.7785324, 0.83101291, 0.27368778, 0.09374625, 0.04166516};

void scale(float *x) {
    for (int i = 0; i < 11; i++) {
        x[i] = (x[i] - mean[i]) / (stdv[i] + 1e-6);
    }
}

// ===== FEATURE EXTRACTION =====
void extractFeatures(float *x) {
    double sum = 0, sqSum = 0, maxv = 0;

    for (int i = 0; i < WINDOW; i++) {
        float v = buffer[i];
        sum += v;
        sqSum += v * v;
        if (abs(v) > maxv) maxv = abs(v);
        vReal[i] = (double)v;
        vImag[i] = 0;
    }

    float meanVal = sum / WINDOW;
    float rms = sqrt(sqSum / WINDOW);
    float var = (sqSum / WINDOW) - (meanVal * meanVal);

    // Skewness & Kurtosis
    float skew = 0, kurt = 0;
    for (int i = 0; i < WINDOW; i++) {
        float diff = buffer[i] - meanVal;
        skew += pow(diff, 3);
        kurt += pow(diff, 4);
    }
    
    x[0] = rms;
    x[1] = var;
    x[2] = (float)maxv;
    x[3] = (skew / WINDOW) / pow(var + 1e-6, 1.5);
    x[4] = (kurt / WINDOW) / pow(var + 1e-6, 2.0);
    x[5] = (float)maxv / (rms + 1e-6);

    // FFT Analysis
    FFT.Windowing(vReal, WINDOW, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, WINDOW, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, WINDOW);

    x[6] = (float)FFT.MajorPeak(vReal, WINDOW, SAMPLING_FREQ);

    // Entropy & Energy Bands
    float totalEnergy = 0;
    float lowE = 0, midE = 0, highE = 0;
    int half = WINDOW / 2;
    int bandSize = half / 3;

    for (int i = 2; i < half; i++) {
        float mag = (float)vReal[i];
        totalEnergy += mag;
        if (i < bandSize) lowE += mag;
        else if (i < 2 * bandSize) midE += mag;
        else highE += mag;
    }

    // Spectral Entropy
    float entropy = 0;
    for (int i = 2; i < half; i++) {
        float p = (float)vReal[i] / (totalEnergy + 1e-6);
        if (p > 0) entropy -= p * log(p);
    }

    x[7] = entropy;
    x[8] = lowE / (totalEnergy + 1e-6);
    x[9] = midE / (totalEnergy + 1e-6);
    x[10] = highE / (totalEnergy + 1e-6);
}

// ===== CLOUD COMMUNICATION =====
void reportToCloud(float *rawFeatures) {
    // This format is parsed by the FastAPI cloud_gateway.py
    Serial.print("raw:");
    for (int i = 0; i < 11; i++) {
        Serial.print(rawFeatures[i], 6);
        if (i < 10) Serial.print(",");
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    if (!IMU.begin()) {
        Serial.println("IMU Init Failed!");
        while (1);
    }
    Serial.println("NANO EDGE READY: CIL MODE ACTIVE");
}

void loop() {
    float ax, ay, az;

    if (IMU.accelerationAvailable()) {
        IMU.readAcceleration(ax, ay, az);
        float mag = sqrt(ax * ax + ay * ay + az * az);
        buffer[bufferIdx++] = mag;

        if (bufferIdx >= WINDOW) {
            bufferIdx = 0;
            float rawFeat[11];
            float scaledFeat[11];

            extractFeatures(rawFeat);
            memcpy(scaledFeat, rawFeat, sizeof(rawFeat));
            scale(scaledFeat);

            // SVM Gatekeeper
            // svm.predict(x) returns 1 for Anomaly, -1 (or 0) for Known
            Serial.print(svm.predict(scaledFeat));
            if (svm.predict(scaledFeat) == 0) {
                reportToCloud(rawFeat);
            } else {
                int prediction = xgb.predict(scaledFeat);
                Serial.print("KNOWN CLASS: ");
                Serial.println(prediction);
            }
        }
    }
}