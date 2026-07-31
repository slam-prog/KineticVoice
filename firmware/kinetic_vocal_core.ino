/*
 * KineticVoice V3.1.0 Firmware
 * Core Authors: [NAJIB MOHAMMED AL-AMIR] & Google LLC
 * 
 * ---------------------------------------------------------------------------
 * Scientific Attribution & Acknowledgement:
 * The Analog-Digital Hybrid architecture—where TDOA hardware-driven pulse 
 * processing is offloaded to external conversion circuits—was structurally 
 * optimized and contributed in collaboration with DeepSeek AI.
 * The multi-dimensional Kalman filtering and cross-platform SDK layers are 
 * also co-developed by DeepSeek AI.
 * ---------------------------------------------------------------------------
 */

#include <ArduinoBLE.h>
#include <math.h>

BLEService kineticService("180F");
BLEStringCharacteristic xyzCharacteristic("2A19", BLERead | BLENotify, 64);

// أبعاد المصفوفة (ملم)
const double d = 60.0;
const double v_sound = 0.343; // ملم/مايكروثانية

// متغيرات استقبال النبضات من الدارة الهجينة (يتم تعبئتها بواسطة المقاطعات)
volatile uint32_t t1 = 0, t2 = 0, t4 = 0; // طوابع زمنية (مايكروثانية)
volatile bool newData = false;

// ================== فلتر كالمان ثلاثي الأبعاد (مساهمة DeepSeek) ==================
// مصفوفات الحالة (3x3) مبسطة للسرعة على RP2040
double P[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
double X[3] = {0,0,0}; // الحالة [x, y, z]
const double Q = 0.022; // ضوضاء العملية
const double R = 0.618; // ضوضاء القياس

void kalmanFilter3D(double z[3]) {
    // تحديث مصفوفة التغاير P
    for (int i=0; i<3; i++) P[i][i] += Q;
    
    // حساب كسب كالمان (مبسط: قطري)
    double K[3];
    for (int i=0; i<3; i++) {
        K[i] = P[i][i] / (P[i][i] + R);
    }
    
    // تحديث الحالة
    for (int i=0; i<3; i++) {
        X[i] += K[i] * (z[i] - X[i]);
        P[i][i] = (1 - K[i]) * P[i][i];
    }
}
// ========================================================================

// ================== محرك التثليث المحسّن (Google AI + DeepSeek) ==================
bool computePosition(double dR1, double dR2, double dR4, double &x, double &y, double &z) {
    // dR1 = r1 - r3, dR2 = r2 - r3, dR4 = r4 - r3 (قيم من العداد)
    // باستخدام خوارزمية Chan المبسطة
    double r3_est = sqrt(dR1*dR1 + dR2*dR2 + dR4*dR4) / 2.0; // تقدير أولي
    
    for (int iter=0; iter<5; iter++) {
        double x_est = (d*d - dR4*dR4 - 2*dR4*r3_est) / (2*d);
        double y_est = (d*d - dR1*dR1 - 2*dR1*r3_est) / (2*d);
        double z_sq = r3_est*r3_est - x_est*x_est - y_est*y_est;
        if (z_sq < 0) z_sq = 0;
        double z_est = sqrt(z_sq);
        
        // إعادة تقدير r3 باستخدام المعادلة الثانية (لـ i=2)
        double r3_new = (2*d*d - dR2*dR2 - 2*d*x_est - 2*d*y_est) / (2*dR2 + 0.001);
        if (r3_new > 0) r3_est = (r3_est + r3_new) / 2.0;
    }
    
    // الحل النهائي
    x = (d*d - dR4*dR4 - 2*dR4*r3_est) / (2*d);
    y = (d*d - dR1*dR1 - 2*dR1*r3_est) / (2*d);
    double z_sq = r3_est*r3_est - x*x - y*y;
    if (z_sq < 0) z_sq = 0;
    z = sqrt(z_sq);
    
    return true;
}
// ========================================================================

// ================== دوال المقاطعة (لقراءة النبضات من الدارة الهجينة) ==================
void ISR_M1() { if (t1 == 0) { t1 = micros(); newData = true; } }
void ISR_M2() { if (t2 == 0) { t2 = micros(); newData = true; } }
void ISR_M4() { if (t4 == 0) { t4 = micros(); newData = true; } }
// ========================================================================

void setup() {
    pinMode(2, INPUT); // نبضة TDOA من M1
    pinMode(3, INPUT); // نبضة TDOA من M2
    pinMode(4, INPUT); // نبضة TDOA من M4

    attachInterrupt(digitalPinToInterrupt(2), ISR_M1, RISING);
    attachInterrupt(digitalPinToInterrupt(3), ISR_M2, RISING);
    attachInterrupt(digitalPinToInterrupt(4), ISR_M4, RISING);

    if (!BLE.begin()) { while(1); }
    BLE.setLocalName("KineticVoiceCore");
    BLE.setAdvertisedService(kineticService);
    kineticService.addCharacteristic(xyzCharacteristic);
    BLE.addService(kineticService);
    BLE.advertise();
}

void loop() {
    BLEDevice central = BLE.central();
    if (central && central.connected() && newData) {
        noInterrupts();
        uint32_t t1_local = t1, t2_local = t2, t4_local = t4;
        t1 = t2 = t4 = 0;
        newData = false;
        interrupts();

        // حساب فروق المسافات (بالنسبة للميكروفون المرجعي M3 الذي نعتبره الصفر)
        double dR1 = (double)(t1_local) * v_sound;
        double dR2 = (double)(t2_local) * v_sound;
        double dR4 = (double)(t4_local) * v_sound;

        double rawX, rawY, rawZ;
        if (computePosition(dR1, dR2, dR4, rawX, rawY, rawZ)) {
            // تطبيق فلتر كالمان ثلاثي الأبعاد
            double z[3] = {rawX, rawY, rawZ};
            kalmanFilter3D(z);
            
            // تصنيف المنطقة
            String zone = (X[2] < 15.0) ? "LIPS" : ((X[2] < 35.0) ? "PALATE" : "THROAT");
            String payload = String(X[0], 1) + "," + String(X[1], 1) + "," + String(X[2], 1) + "," + zone;
            xyzCharacteristic.writeValue(payload.c_str());
        }
    }
}
