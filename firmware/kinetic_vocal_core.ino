/*
 * KineticVoice V3.1.1 Firmware
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

// ==================== الثوابت الهندسية والفيزيائية ====================
const double d = 60.0;                     // المسافة بين الميكروفونات (ملم)
const double SPEED_OF_SOUND = 0.343;       // مم/مايكروثانية
const double EPSILON = 0.00001;            // قيمة صغيرة لمنع القسمة على صفر

// ==================== إعدادات فلتر كالمان ====================
// قيم مُحسّنة لتحقيق توازن بين الاستجابة السريعة والاستقرار
const double Q = 0.022;    // ضوضاء العملية (منخفضة لافتراض حركة بطيئة لمصدر الصوت)
const double R = 0.618;    // ضوضاء القياس (عالية نسبياً لمراعاة تشويش الميكروفونات)

// ==================== متغيرات الحالة العامة ====================
// دوال المقاطعة تملأ هذه المتغيرات (يجب أن تكون volatile)
volatile uint32_t t1 = 0, t2 = 0, t4 = 0;
volatile bool newData = false;
volatile bool dataReady = false;     // علم إضافي للإشارة إلى اكتمال جميع النبضات

// متغيرات فلتر كالمان (حالة ثنائية الأبعاد)
double P[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
double X[3] = {0,0,0}; // متجه الحالة [x, y, z]

// ==================== خدمة وخاصية البلوتوث ====================
BLEService kineticService("180F");
BLEStringCharacteristic xyzCharacteristic("2A19", BLERead | BLENotify, 64);

// ==================== النماذج الأولية للدوال (Prototypes) ====================
void kalmanFilter3D(double z[3]);
bool computePosition(double dR1, double dR2, double dR4, double &x, double &y, double &z);
String classifyZone(double x, double y, double z);
void resetTimers();

// ==================== دوال المقاطعة (ISR) ====================
void ISR_M1() {
    if (t1 == 0) {
        t1 = micros();
        newData = true;
    }
}
void ISR_M2() {
    if (t2 == 0) {
        t2 = micros();
        newData = true;
    }
}
void ISR_M4() {
    if (t4 == 0) {
        t4 = micros();
        newData = true;
    }
}

// ==================== دالة الإعداد ====================
void setup() {
    // تكوين دبابيس المقاطعات (حسب توصيل الدارة الهجينة)
    pinMode(2, INPUT); // نبضة TDOA من M1
    pinMode(3, INPUT); // نبضة TDOA من M2
    pinMode(4, INPUT); // نبضة TDOA من M4

    // ربط المقاطعات بالحواف الصاعدة
    attachInterrupt(digitalPinToInterrupt(2), ISR_M1, RISING);
    attachInterrupt(digitalPinToInterrupt(3), ISR_M2, RISING);
    attachInterrupt(digitalPinToInterrupt(4), ISR_M4, RISING);

    // بدء الاتصال التسلسلي للتصحيح (Debug)
    Serial.begin(115200);

    // بدء البلوتوث مع حماية ضد الفشل
    if (!BLE.begin()) {
        Serial.println("فشل بدء البلوتوث!");
        while (1);
    }

    BLE.setLocalName("KineticVoiceCore");
    BLE.setAdvertisedService(kineticService);
    kineticService.addCharacteristic(xyzCharacteristic);
    BLE.addService(kineticService);
    BLE.advertise();

    Serial.println("تم تهيئة KineticVoice V3.1.1 بنجاح.");
    Serial.println("في انتظار الاتصال...");
}

// ==================== الحلقة الرئيسية ====================
void loop() {
    // التحقق من وجود جهاز مركزي متصل
    BLEDevice central = BLE.central();
    if (central && central.connected()) {
        // التحقق من توفر بيانات جديدة
        if (newData) {
            // تعطيل المقاطعات مؤقتاً لقراءة آمنة للقيم
            noInterrupts();
            uint32_t t1_local = t1;
            uint32_t t2_local = t2;
            uint32_t t4_local = t4;
            // إعادة تعيين المتغيرات لمنع القراءة المكررة
            t1 = 0;
            t2 = 0;
            t4 = 0;
            newData = false;
            interrupts();

            // التأكد من استلام جميع النبضات الثلاث
            if (t1_local > 0 && t2_local > 0 && t4_local > 0) {
                // حساب فروق المسافات (بالنسبة للميكروفون المرجعي M3 الذي نعتبره الصفر)
                double dR1 = (double)(t1_local) * SPEED_OF_SOUND;
                double dR2 = (double)(t2_local) * SPEED_OF_SOUND;
                double dR4 = (double)(t4_local) * SPEED_OF_SOUND;

                // حساب الإحداثيات الخام
                double rawX, rawY, rawZ;
                if (computePosition(dR1, dR2, dR4, rawX, rawY, rawZ)) {
                    // تطبيق فلتر كالمان ثلاثي الأبعاد
                    double z[3] = {rawX, rawY, rawZ};
                    kalmanFilter3D(z);

                    // تصنيف المنطقة النطقية
                    String zone = classifyZone(X[0], X[1], X[2]);

                    // بناء الحزمة النصية للإرسال
                    String payload = String(X[0], 1) + "," +
                                    String(X[1], 1) + "," +
                                    String(X[2], 1) + "," +
                                    zone;

                    // إرسال البيانات عبر البلوتوث
                    xyzCharacteristic.writeValue(payload.c_str());

                    // إرسال البيانات عبر المنفذ التسلسلي للتصحيح (Debug Mode)
                    Serial.print("Raw: ");
                    Serial.print(rawX, 1); Serial.print(",");
                    Serial.print(rawY, 1); Serial.print(",");
                    Serial.print(rawZ, 1);
                    Serial.print(" | Filtered: ");
                    Serial.print(X[0], 1); Serial.print(",");
                    Serial.print(X[1], 1); Serial.print(",");
                    Serial.print(X[2], 1);
                    Serial.print(" | Zone: ");
                    Serial.println(zone);
                } else {
                    // في حال فشل الحساب، إرسال بيانات افتراضية للتنبيه
                    xyzCharacteristic.writeValue("0.0,0.0,0.0,ERROR");
                }
            }
        }
    } else {
        // في حال عدم وجود اتصال، انتظار مع استهلاك طاقة منخفض
        delay(100);
    }
}

// ==================== خوارزمية التثليث المحسّنة (Modified Chan's TDOA Solver) ====================
// مساهمة: Google AI (الهيكل النظري) + DeepSeek AI (تحسين التقارب)
bool computePosition(double dR1, double dR2, double dR4, double &x, double &y, double &z) {
    // dR1 = r1 - r3, dR2 = r2 - r3, dR4 = r4 - r3 (قيم من العداد)
    // باستخدام خوارزمية Chan المبسطة مع 5 تكرارات لتحسين الدقة

    // تقدير أولي لـ r3 (المسافة المطلقة للمرجع M3)
    double r3_est = sqrt(dR1 * dR1 + dR2 * dR2 + dR4 * dR4) / 2.0;

    // منع القيم غير المنطقية
    if (r3_est < 0.001) return false;

    for (int iter = 0; iter < 6; iter++) { // 6 تكرارات لضمان التقارب
        double x_est = (d * d - dR4 * dR4 - 2 * dR4 * r3_est) / (2 * d);
        double y_est = (d * d - dR1 * dR1 - 2 * dR1 * r3_est) / (2 * d);
        double z_sq = r3_est * r3_est - x_est * x_est - y_est * y_est;

        // حماية من الجذر التربيعي السالب
        if (z_sq < 0) z_sq = 0;
        double z_est = sqrt(z_sq);

        // إعادة تقدير r3 باستخدام المعادلة الثانية (لـ i=2) مع حماية القسمة
        double denominator = 2 * dR2 + EPSILON;
        double r3_new = (2 * d * d - dR2 * dR2 - 2 * d * x_est - 2 * d * y_est) / denominator;

        // تحديث التقدير بمتوسط مرجح لتسريع التقارب
        if (r3_new > 0) {
            r3_est = (r3_est + r3_new) / 2.0;
        }

        // الحل النهائي بعد آخر تكرار
        if (iter == 5) {
            x = x_est;
            y = y_est;
            z = z_est;
        }
    }

    // التحقق النهائي من صحة الإحداثيات (منع القيم الشاذة)
    if (isnan(x) || isnan(y) || isnan(z) || x > 100 || y > 100 || z > 100) {
        return false;
    }

    return true;
}

// ==================== فلتر كالمان ثلاثي الأبعاد ====================
// مساهمة: DeepSeek AI (التنفيذ البرمجي) + Google AI (المراجعة الرياضية)
void kalmanFilter3D(double z[3]) {
    // تحديث مصفوفة التغاير (قطرية لتقليل التعقيد الحسابي)
    for (int i = 0; i < 3; i++) {
        P[i][i] += Q;
    }

    // حساب كسب كالمان (قطري)
    double K[3];
    for (int i = 0; i < 3; i++) {
        K[i] = P[i][i] / (P[i][i] + R);
    }

    // تحديث متجه الحالة
    for (int i = 0; i < 3; i++) {
        X[i] += K[i] * (z[i] - X[i]);
        P[i][i] = (1 - K[i]) * P[i][i];
    }
}

// ==================== تصنيف المنطقة النطقية ====================
// مساهمة: Google AI (التصنيف النظري) + DeepSeek AI (التوسيع العملي)
String classifyZone(double x, double y, double z) {
    // نستخدم الإحداثيات المفلترة (Y للارتفاع، Z للعمق التشريحي)
    // تم ضبط الحدود تجريبياً لتناسب تشريح الفم البشري المتوسط

    if (y > 45.0 && z < 15.0) {
        return "LIPS";         // الشفاه (B, P, M)
    } else if (y > 30.0 && z < 25.0) {
        return "DENTAL";       // الأسنان واللثة (T, D, N, S, Z)
    } else if (y > 20.0 && z < 35.0) {
        return "ALVEOLAR";     // الغار السنخي (R, SH, CH, J)
    } else if (y > 15.0 && z < 45.0) {
        return "PALATAL";      // سقف الحلق (Y, soft G, etc.)
    } else if (y > 8.0 && z < 55.0) {
        return "VELAR";        // اللهاة (K, G, NG)
    } else if (z >= 55.0) {
        return "THROAT";       // البلعوم والحنجرة (H, glottal sounds)
    } else {
        return "UNKNOWN";      // منطقة غير مصنفة
    }
}

// ==================== إعادة تعيين المؤقتات (للاستخدام المستقبلي) ====================
void resetTimers() {
    noInterrupts();
    t1 = 0;
    t2 = 0;
    t4 = 0;
    newData = false;
    interrupts();
}
