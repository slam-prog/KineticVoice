/*
 * ⚕️ MEDICAL DISCLAIMER:
 * This software is part of a research platform and is NOT a certified medical device.
 * It should not be used for diagnostic or therapeutic purposes without professional supervision.
 * See DISCLAIMER.md for full terms.
 */

/*
 * KineticVoice V3.1.3 - Production Firmware (Ultimate Stability)
 * Core Authors: [NAJIB MOHAMMED AL-AMIR] & Google AI & DeepSeek AI
 * ---------------------------------------------------------------------------
 * This version addresses three critical silent bugs:
 * 1. TDOA absolute time assumption (now uses relative time differences).
 * 2. BLE MTU limitation (now uses a compact binary payload).
 * 3. Chan's algorithm divergence (improved initial estimation & EPSILON).
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

// ==================== خدمة وخاصية البلوتوث (حزمة ثنائية) ====================
BLEService kineticService("180F");
BLECharacteristic xyzCharacteristic("2A19", BLERead | BLENotify, 7); // 7 بايت فقط

// ==================== النماذج الأولية للدوال ====================
void kalmanFilter3D(double z[3]);
bool computePosition(double dR1, double dR2, double dR4, double &x, double &y, double &z);
uint8_t encodeZone(String zone);
String decodeZone(uint8_t byte);

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

    Serial.println("KineticVoice V3.1.3 Initialized.");
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
            // ========== التصحيح 1: حساب TDOA النسبي ==========
            uint32_t t_min = min(t1_local, min(t2_local, t4_local));
            double dR1 = (double)(t1_local - t_min) * SPEED_OF_SOUND;
            double dR2 = (double)(t2_local - t_min) * SPEED_OF_SOUND;
            double dR4 = (double)(t4_local - t_min) * SPEED_OF_SOUND;

            double rawX, rawY, rawZ;
            if (computePosition(dR1, dR2, dR4, rawX, rawY, rawZ)) {
                double z[3] = {rawX, rawY, rawZ};
                kalmanFilter3D(z);

                String zone = classifyZone(X[0], X[1], X[2]);
                uint8_t zoneByte = encodeZone(zone);

                // ========== التصحيح 2: حزمة ثنائية مضغوطة (7 بايت) ==========
                // التنسيق: [X_H, X_L, Y_H, Y_L, Z_H, Z_L, ZONE_BYTE]
                uint8_t payload[7];
                int16_t x_int = (int16_t)(X[0] * 10); // دقة 0.1 مم
                int16_t y_int = (int16_t)(X[1] * 10);
                int16_t z_int = (int16_t)(X[2] * 10);

                payload[0] = (uint8_t)(x_int >> 8);
                payload[1] = (uint8_t)(x_int & 0xFF);
                payload[2] = (uint8_t)(y_int >> 8);
                payload[3] = (uint8_t)(y_int & 0xFF);
                payload[4] = (uint8_t)(z_int >> 8);
                payload[5] = (uint8_t)(z_int & 0xFF);
                payload[6] = zoneByte;

                xyzCharacteristic.writeValue(payload, 7);

                // وضع التصحيح (Debug Mode) - اختياري
                Serial.print("X:"); Serial.print(X[0], 1);
                Serial.print(",Y:"); Serial.print(X[1], 1);
                Serial.print(",Z:"); Serial.print(X[2], 1);
                Serial.print(",Zone:"); Serial.println(zone);
            }
        }
    }
}

// ==================== خوارزمية التثليث المحسّنة (Modified Chan's Solver) ====================
bool computePosition(double dR1, double dR2, double dR4, double &x, double &y, double &z) {
    // ========== التصحيح 3: تحسين التقدير الأولي ==========
    double r3_est = sqrt(dR1 * dR1 + dR2 * dR2 + dR4 * dR4) / 2.0;
    if (r3_est < 0.001) {
        r3_est = (dR1 + dR2 + dR4) / 3.0 + 10.0; // تقدير افتراضي
    }

    for (int iter = 0; iter < 6; iter++) {
        double x_est = (d * d - dR4 * dR4 - 2 * dR4 * r3_est) / ((2 * d) + EPSILON);
        double y_est = (d * d - dR1 * dR1 - 2 * dR1 * r3_est) / ((2 * d) + EPSILON);
        double z_sq = r3_est * r3_est - x_est * x_est - y_est * y_est;

        double z_est = 0.0;
        if (z_sq > 0.0) {
            z_est = sqrt(z_sq);
        }

        double denominator = (2 * dR2) + EPSILON;
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

// ==================== تشفير المناطق إلى بايت واحد ====================
uint8_t encodeZone(String zone) {
    if (zone == "LIPS") return 1;
    else if (zone == "DENTAL") return 2;
    else if (zone == "ALVEOLAR") return 3;
    else if (zone == "PALATAL") return 4;
    else if (zone == "VELAR") return 5;
    else if (zone == "THROAT") return 6;
    else return 0;
}

// ==================== فك تشفير المنطقة (للاستخدام في الطرف الآخر) ====================
String decodeZone(uint8_t byte) {
    switch(byte) {
        case 1: return "LIPS";
        case 2: return "DENTAL";
        case 3: return "ALVEOLAR";
        case 4: return "PALATAL";
        case 5: return "VELAR";
        case 6: return "THROAT";
        default: return "UNKNOWN";
    }
}
