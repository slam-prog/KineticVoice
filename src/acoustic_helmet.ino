/**
 * @file acoustic_helmet.ino
 * @brief Wireless 2D Phoneme Localization using Built-in BLE & Trilateration.
 * @note Architecture designed via Human & Google AI collaboration.
 * @version 1.2.0 (Final Production-Ready)
 */

#include <ArduinoBLE.h>

// 1. الإحداثيات الفيزيائية الثابتة للميكروفونات الأربعة على الخوذة (بالمليمتر)
const float x1 = -30.0, y1 =  30.0; // M1: أعلى اليسار
const float x2 =  30.0, y2 =  30.0; // M2: أعلى اليمين
const float x3 = -30.0, y3 = -30.0; // M3: أسفل اليسار
const float x4 =  30.0, y4 = -30.0; // M4: أسفل اليمين

// 2. دبابيس المقاطعة السريعة المتصلة بالميكروفونات
const int PIN_M1 = 2;   const int PIN_M2 = 3;
const int PIN_M3 = 18;  const int PIN_M4 = 19;

// طوابع زمنية متطايرة لالتقاط النبضات بدقة المايكروثانية
volatile unsigned long t1 = 0, t2 = 0, t3 = 0, t4 = 0;
const float SPEED_OF_SOUND = 0.343; // مم لكل مايكروثانية
const unsigned long DEBOUNCE_DELAY = 200000; // منع الصدى (200ms)
unsigned long lastTriggerTime = 0;

// 3. إعدادات خدمة البلوتوث اللاسلكية ذات الاستهلاك المنخفض (BLE)
BLEService helmetService("180F"); 
BLEStringCharacteristic jsonCharacteristic("2A19", BLERead | BLENotify, 128);

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_M1, INPUT); pinMode(PIN_M2, INPUT);
  pinMode(PIN_M3, INPUT); pinMode(PIN_M4, INPUT);
  
  // تفعيل المقاطعات عند صعود الموجة الصوتية
  attachInterrupt(digitalPinToInterrupt(PIN_M1), ISR_M1, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_M2), ISR_M2, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_M3), ISR_M3, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_M4), ISR_M4, RISING);

  if (!BLE.begin()) {
    while (1); // إيقاف إن لم تعمل شريحة البلوتوث
  }

  BLE.setLocalName("AcousticHelmet");
  BLE.setAdvertisedService(helmetService);
  helmetService.addCharacteristic(jsonCharacteristic);
  BLE.addService(helmetService);
  jsonCharacteristic.writeValue("{}");
  BLE.advertise();
}

void loop() {
  BLEDevice central = BLE.central();

  // لا يتم تفعيل حساب الرياضيات إلا عند وجود اتصال لاسلكي نشط لتوفير طاقة البطارية
  if (central && central.connected()) {
    if (t1 > 0 && t2 > 0 && t3 > 0 && t4 > 0) {
      
      unsigned long t_min = min(min(t1, t2), min(t3, t4));
      
      // حساب مسافات الارتطام الصوتية (TDOA)
      float r1 = (t1 - t_min) * SPEED_OF_SOUND;
      float r2 = (t2 - t_min) * SPEED_OF_SOUND;
      float r3 = (t3 - t_min) * SPEED_OF_SOUND;
      
      // حل مصفوفة القطع الزائد رياضياً وتحديد الموقع الثلاثي (Trilateration)
      float A = 2 * x2 - 2 * x1;
      float B = 2 * y2 - 2 * y1;
      float C = sq(r1) - sq(r2) - sq(x1) + sq(x2) - sq(y1) + sq(y2);
      
      float D_val = 2 * x3 - 2 * x2;
      float E     = 2 * y3 - 2 * y2;
      float F_val = sq(r2) - sq(r3) - sq(x2) + sq(x3) - sq(y2) + sq(y3);
      
      float denominator = (A * E - D_val * B);
      
      if (abs(denominator) > 0.001) {
        float exactX = (C * E - F_val * B) / denominator;
        float exactY = (A * F_val - D_val * C) / denominator;

        // تصنيف المخرج النطقي هندسياً
        String zone = "UNKNOWN";
        if (exactY > 15.0 && abs(exactX) < 10.0) zone = "LABIAL_LIPS";
        else if (exactY >= 0.0 && exactY <= 15.0 && abs(exactX) < 5.0) zone = "DENTAL_ALVEOLAR";
        else if (exactY < 0.0 && exactY > -15.0) zone = "PALATAL_MID_MOUTH";
        else if (exactY <= -15.0) zone = "PHARYNGEAL_THROAT";

        // ميزة إضافية: قراءة مستوى بطارية الخوذة (منفذ A6 التناظري)
        int batteryPercentage = map(constrain((analogRead(A6) * 3.3 / 1023.0) * 1.2 * 100, 340, 420), 340, 420, 0, 100);

        // بناء حزمة الـ JSON اللاسلكية النهائية وبثها
        String jsonPayload = "{\"X\":" + String(exactX, 2) + ",\"Y\":" + String(exactY, 2) + ",\"ZONE\":\"" + zone + "\",\"BATTERY\":" + String(batteryPercentage) + "}";
        jsonCharacteristic.writeValue(jsonPayload);
      }

      delay(200); // تجميد لحظي لتفادي ارتدادات الموجة الصامتة للمقطع اللفظي
      noInterrupts();
      t1 = 0; t2 = 0; t3 = 0; t4 = 0;
      interrupts();
    }
  }
}

void ISR_M1() { if (t1 == 0 && (micros() - lastTriggerTime > DEBOUNCE_DELAY)) t1 = micros(); }
void ISR_M2() { if (t2 == 0 && (micros() - lastTriggerTime > DEBOUNCE_DELAY)) t2 = micros(); }
void ISR_M3() { if (t3 == 0 && (micros() - lastTriggerTime > DEBOUNCE_DELAY)) t3 = micros(); }
void ISR_M4() { if (t4 == 0 && (micros() - lastTriggerTime > DEBOUNCE_DELAY)) t4 = micros(); }
