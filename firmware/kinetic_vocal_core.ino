/*
 * KineticVoice V3.1.2 - Production Firmware (Hybrid Optimized)
 * Core Authors: [NAJIB MOHAMMED AL-AMIR] & Google AI & DeepSeek AI
 * ---------------------------------------------------------------------------
 * This version integrates the NaN protection (Google AI) with the high-speed
 * interrupt-driven architecture, advanced trilateration, and 3D Kalman filter
 * (DeepSeek AI), delivering both stability and peak performance.
 * ---------------------------------------------------------------------------
 */

#include <ArduinoBLE.h>
#include <math.h>

// ==================== الثوابت الهندسية والفيزيائية ====================
const double d = 60.0;
const double SPEED_OF_SOUND = 0.343; // مم/مايكروثانية
const double EPSILON = 0.00001;

// ==================== إعدادات فلتر كالمان ثلاثي الأبعاد ====================
const double Q = 0.022;
const double R = 0.618;
double P[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
double X[3] = {0,0,0};

// ==================== متغيرات المقاطعة (النهج اللحظي) ====================
volatile uint32_t t1 = 0, t2 = 0, t4 = 0;
volatile bool newData = false;

// ==================== خدمة وخاصية البلوتوث ====================
BLEService kineticService("180F");
BLEStringCharacteristic xyzCharacteristic("2A19", BLERead | BLENotify, 64);

// ==================== النماذج الأولية للدوال ====================
void kalmanFilter3D(double z[3]);
bool computePosition(double dR1, double dR2, double dR4, double &x, double &y, double &z);
String classifyZone(double x, double y, double z);

// ==================== دوال المقاطعة (ISR) ====================
void ISR_M1() { if (t1 == 0) { t1 = micros(); newData = true; } }
void ISR_M2() { if (t2 == 0) { t2 = micros(); newData = true; } }
void ISR_M4() { if (t4 == 0) { t4 = micros(); newData = true; } }

// ==================== دالة الإعداد ====================
void setup() {
    Serial.begin(115200);

    pinMode(2, INPUT);
    pinMode(3, INPUT);
    pinMode(4, INPUT);

    attachInterrupt(digitalPinToInterrupt(2), ISR_M1, RISING);
    attachInterrupt(digitalPinToInterrupt(3), ISR_M2, RISING);
    attachInterrupt(digitalPinToInterrupt(4), ISR_M4, RISING);

    if (!BLE.begin()) {
        Serial.println("Error: BLE hardware failure!");
        while (1);
    }

    BLE.setLocalName("KineticVoiceCore");
    BLE.setAdvertisedService(kineticService);
    kineticService.addCharacteristic(xyzCharacteristic);
    BLE.addService(kineticService);
    BLE.advertise();

    Serial.println("KineticVoice V3.1.2 Initialized.");
}

// ==================== الحلقة الرئيسية ====================
void loop() {
    BLEDevice central = BLE.central();
    if (central && central.connected() && newData) {
        noInterrupts();
        uint32_t t1_local = t1, t2_local = t2, t4_local = t4;
        t1 = t2 = t4 = 0;
        newData = false;
        interrupts();

        if (t1_local > 0 && t2_local > 0 && t4_local > 0) {
            double dR1 = (double)(t1_local) * SPEED_OF_SOUND;
            double dR2 = (double)(t2_local) * SPEED_OF_SOUND;
            double dR4 = (double)(t4_local) * SPEED_OF_SOUND;

            double rawX, rawY, rawZ;
            if (computePosition(dR1, dR2, dR4, rawX, rawY, rawZ)) {
                double z[3] = {rawX, rawY, rawZ};
                kalmanFilter3D(z);

                String zone = classifyZone(X[0], X[1], X[2]);
                String payload = String(X[0], 1) + "," + String(X[1], 1) + "," + String(X[2], 1) + "," + zone;

                xyzCharacteristic.writeValue(payload.c_str());

                // وضع التصحيح (Debug Mode)
                Serial.print("Raw: "); Serial.print(rawX, 1); Serial.print(","); Serial.print(rawY, 1); Serial.print(","); Serial.print(rawZ, 1);
                Serial.print(" | Filtered: "); Serial.print(X[0], 1); Serial.print(","); Serial.print(X[1], 1); Serial.print(","); Serial.print(X[2], 1);
                Serial.print(" | Zone: "); Serial.println(zone);
            }
        }
    }
}

// ==================== خوارزمية التثليث المحسّنة (Modified Chan's Solver) ====================
bool computePosition(double dR1, double dR2, double dR4, double &x, double &y, double &z) {
    double r3_est = sqrt(dR1 * dR1 + dR2 * dR2 + dR4 * dR4) / 2.0;
    if (r3_est < 0.001) return false;

    for (int iter = 0; iter < 6; iter++) {
        double x_est = (d * d - dR4 * dR4 - 2 * dR4 * r3_est) / ((2 * d) + EPSILON);
        double y_est = (d * d - dR1 * dR1 - 2 * dR1 * r3_est) / ((2 * d) + EPSILON);
        double z_sq = r3_est * r3_est - x_est * x_est - y_est * y_est;

        // 🔥 تصحيح Google AI: منع الجذر التربيعي السالب (NaN Protection)
        double z_est = 0.0;
        if (z_sq > 0.0) {
            z_est = sqrt(z_sq);
        }

        double denominator = 2 * dR2 + EPSILON;
        double r3_new = (2 * d * d - dR2 * dR2 - 2 * d * x_est - 2 * d * y_est) / denominator;
        if (r3_new > 0) {
            r3_est = (r3_est + r3_new) / 2.0;
        }

        if (iter == 5) {
            x = x_est;
            y = y_est;
            z = z_est;
        }
    }

    if (isnan(x) || isnan(y) || isnan(z) || x > 100 || y > 100 || z > 100) {
        return false;
    }
    return true;
}

// ==================== فلتر كالمان ثلاثي الأبعاد ====================
void kalmanFilter3D(double z[3]) {
    for (int i = 0; i < 3; i++) {
        P[i][i] += Q;
    }

    double K[3];
    for (int i = 0; i < 3; i++) {
        K[i] = P[i][i] / (P[i][i] + R);
    }

    for (int i = 0; i < 3; i++) {
        X[i] += K[i] * (z[i] - X[i]);
        P[i][i] = (1 - K[i]) * P[i][i];
    }
}

// ==================== تصنيف المنطقة النطقية (6 مناطق) ====================
String classifyZone(double x, double y, double z) {
    if (y > 45.0 && z < 15.0) return "LIPS";
    else if (y > 30.0 && z < 25.0) return "DENTAL";
    else if (y > 20.0 && z < 35.0) return "ALVEOLAR";
    else if (y > 15.0 && z < 45.0) return "PALATAL";
    else if (y > 8.0 && z < 55.0) return "VELAR";
    else if (z >= 55.0) return "THROAT";
    else return "UNKNOWN";
}

