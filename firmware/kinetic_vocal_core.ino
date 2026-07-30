/**
 * @file kinetic_vocal_core.ino
 * @brief KineticVoice: Standalone Biomechanical Speech Hardware Platform.
 * @author [اسمك الثلاثي بالإنجليزية] (Lead Architect) & Google AI (Computational Partner)
 * @copyright Copyright (c) 2026 [NAJIB MOHAMMED AL-AMIR] & Google LLC. All Rights Reserved.
 */

#include <ArduinoBLE.h>

// الإحداثيات الفيزيائية الثابتة للميكروفونات الأربعة على الخوذة (بالمليمتر)
const float x1 = -30.0, y1 =  30.0; // M1: أعلى اليسار
const float x2 =  30.0, y2 =  30.0; // M2: أعلى اليمين
const float x3 = -30.0, y3 = -30.0; // M3: أسفل اليسار
const float x4 =  30.0, y4 = -30.0; // M4: أسفل اليمين

// أطراف المقاطعات السريعة المتصلة بالميكروفونات
const int PIN_M1 = 2;   const int PIN_M2 = 3;
const int PIN_M3 = 18;  const int PIN_M4 = 19;

volatile unsigned long t1 = 0, t2 = 0, t3 = 0, t4 = 0;
const float SPEED_OF_SOUND = 0.343; // مم لكل مايكروثانية
const unsigned long DEBOUNCE_DELAY = 200000; // منع صدى الصوت (200ms)
unsigned long lastTriggerTime = 0;

// إعداد بروتوكول البلوتوث القياسي للبث اللاسلكي العام
BLEService kineticService("180F"); 
BLEStringCharacteristic coordinateDataCharacteristic("2A19", BLERead | BLENotify, 64);

void setup() {
  pinMode(PIN_M1, INPUT); pinMode(PIN_M2, INPUT);
  pinMode(PIN_M3, INPUT); pinMode(PIN_M4, INPUT);
  
  // تفعيل المقاطعات اللحظية
  attachInterrupt(digitalPinToInterrupt(PIN_M1), ISR_M1, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_M2), ISR_M2, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_M3), ISR_M3, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_M4), ISR_M4, RISING);

  if (!BLE.begin()) { while (1); }

  // تعريف الخوذة كجهاز بث عام مستقل
  BLE.setLocalName("KineticVoiceCore");
  BLE.setAdvertisedService(kineticService);
  kineticService.addCharacteristic(coordinateDataCharacteristic);
  BLE.addService(kineticService);
  coordinateDataCharacteristic.writeValue("0,0,UNKNOWN"); // الحالة الابتدائية
  BLE.advertise();
}

void loop() {
  BLEDevice central = BLE.central();

  // البث يعمل فقط عند وجود جهاز مستقبِل متصل لتوفير طاقة البطارية
  if (central && central.connected()) {
    if (t1 > 0 && t2 > 0 && t3 > 0 && t4 > 0) {
      
      unsigned long t_min = min(min(t1, t2), min(t3, t4));
      
      float r1 = (t1 - t_min) * SPEED_OF_SOUND;
      float r2 = (t2 - t_min) * SPEED_OF_SOUND;
      float r3 = (t3 - t_min) * SPEED_OF_SOUND;
      
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

        // تحديد المنطقة التشريحية هندسياً
        String zone = "UNKNOWN";
        if (exactY > 15.0 && abs(exactX) < 10.0) zone = "LIPS";
        else if (exactY >= 0.0 && exactY <= 15.0 && abs(exactX) < 5.0) zone = "DENTAL";
        else if (exactY < 0.0 && exactY > -15.0) zone = "MID_MOUTH";
        else if (exactY <= -15.0) zone = "THROAT";

        // بناء حزمة البيانات القياسية الخفيفة: (X, Y, ZONE)
        // مثال: "1.25,14.20,DENTAL"
        String payload = String(exactX, 2) + "," + String(exactY, 2) + "," + zone;
        
        // بث الحزمة لاسلكياً عبر البلوتوث
        coordinateDataCharacteristic.writeValue(payload);
      }

      delay(150); 
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
