# دليل مطور منصة KineticVoice
## Developer Guide for KineticVoice Platform

**الإصدار:** v3.1.3 | **التاريخ:** 6 أغسطس 2026

---

## 1. مقدمة (Introduction)

مرحباً بك في دليل مطور منصة **KineticVoice**! هذا الدليل موجه للمطورين والمهندسين الذين يرغبون في فهم الكود البرمجي، تعديله، أو إضافة ميزات جديدة. إذا كنت مستخدماً نهائياً وتبحث عن تعليمات التشغيل، يُرجى الرجوع إلى [دليل المستخدم](./USER_MANUAL.md).

**الجمهور المستهدف**: هذا الدليل موجه للمطورين (المهندسين، الباحثين، المساهمين في المصادر المفتوحة) الذين لديهم خبرة في:
- لغة C++ (لتطوير الكود المضمن).
- بيئة Arduino IDE أو PlatformIO.
- أساسيات معالجة الإشارات الصوتية (اختياري، لكن مفيد).
- لغة Python (لتطوير أدوات التكامل).

---

## 2. هيكل المشروع (Project Structure)

يتبع المستودع الهيكل التالي:
KineticVoice/
├── firmware/
│ └── kinetic_vocal_core.ino # الكود الأساسي للنظام (v3.1.3)
├── examples/
│ ├── v4_experimental/
│ │ └── compute_position_v4.cpp # الكود التجريبي للإصدار v4.0
│ └── integration/ # (قريباً) أمثلة التكامل مع APIs
├── docs/ # (قريباً) التوثيق المتقدم
├── python/ # (قريباً) مكتبة Python الرسمية
├── hardware/ # (قريباً) مخططات الدوائر وملفات التصميم
├── README.md
├── USER_MANUAL.md
├── DEVELOPER_GUIDE.md 
├── CONTRIBUTING.md
├── LICENSE
└── ...

## 3. بيئة التطوير (Development Environment)

### 3.1. الأدوات الموصى بها

- **Arduino IDE** (الإصدار 1.8.19 أو أحدث) مع حزمة **Raspberry Pi Pico/RP2040**.
- **PlatformIO** (اختياري، للمطورين المحترفين) مع تكوين `platformio.ini` مناسب.
- **مكتبة ArduinoBLE** (تُثبت عبر مدير المكتبات).

### 3.2. إعداد البيئة من الصفر

1.  **تثبيت Arduino IDE** من [الموقع الرسمي](https://www.arduino.cc/en/software).
2.  **إضافة حزمة RP2040**:
    - اذهب إلى `File → Preferences`.
    - أضف الرابط التالي إلى `Additional Boards Manager URLs`:
    - https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
    - - اذهب إلى `Tools → Board → Boards Manager`، ابحث عن `Raspberry Pi Pico` وقم بتثبيتها.
3.  **تثبيت مكتبة ArduinoBLE**:
- اذهب إلى `Sketch → Include Library → Manage Libraries`.
- ابحث عن `ArduinoBLE` وقم بتثبيتها.

---

## 4. هيكل الكود البرمجي (Code Architecture)

الكود الأساسي في `kinetic_vocal_core.ino` مقسم إلى الوحدات التالية:

### 4.1. الثوابت والتعريفات (Constants & Definitions)

```cpp
const double d = 60.0;                // المسافة بين الميكروفونات (مم)
const double SPEED_OF_SOUND = 0.343;  // سرعة الصوت (مم/مايكروثانية)
const double EPSILON = 0.00001;       // حماية من القسمة على صفر
4.2. متغيرات المقاطعة (Interrupt Variables)
تُستخدم لحفظ التوقيتات عند وصول الصوت إلى كل ميكروفون.
cpp
void ISR_M1() { if (t1 == 0) { t1 = micros(); newData = true; } }
void ISR_M2() { if (t2 == 0) { t2 = micros(); newData = true; } }
void ISR_M4() { if (t4 == 0) { t4 = micros(); newData = true; } }
4.4. محرك التثليث (Trilateration Engine)
دالة computePosition تحسب الإحداثيات (X, Y, Z) باستخدام خوارزمية Chan المعدلة مع 6 تكرارات.
cpp
bool computePosition(double dR1, double dR2, double dR4, double &x, double &y, double &z) {
    // 1. تقدير أولي لـ r3
    // 2. حلقة تكرارية لتحسين الدقة
    // 3. حساب Z مع حماية من الجذر التربيعي السالب
}
4.5. فلتر كالمان ثلاثي الأبعاد (3D Kalman Filter)
دالة kalmanFilter3D تقوم بتنعيم الإحداثيات وتقليل الضوضاء.
cpp
void kalmanFilter3D(double z[3]) {
    // 1. تحديث مصفوفة التغاير
    // 2. حساب كسب كالمان
    // 3. تحديث متجه الحالة
}
4.6. تصنيف المنطقة التشريحية (Zone Classification)
دالة classifyZone تُصنف الإحداثيات ضمن 6 مناطق تشريحية.
cpp
String classifyZone(double x, double y, double z) {
    if (y > 45.0 && z < 15.0) return "LIPS";
    else if (y > 30.0 && z < 25.0) return "DENTAL";
    // ... إلخ
    else return "UNKNOWN";
}
4.7. البلوتوث (BLE Stack)
يُستخدم لإرسال البيانات لاسلكياً عبر خاصية 2A19.

cpp
BLEService kineticService("180F");
BLEStringCharacteristic xyzCharacteristic("2A19", BLERead | BLENotify, 7);
5. تخصيص النظام (Customization)
5.1. تغيير دبابيس الميكروفونات
يمكنك تعديل الدبابيس في بداية الكود:

cpp
const int PIN_M1 = 2;
const int PIN_M2 = 3;
const int PIN_M4 = 4;
5.2. ضبط حدود المناطق التشريحية
في دالة classifyZone، يمكنك تعديل القيم الحدية لتناسب تشريح المستخدم:

cpp
if (y > 45.0 && z < 15.0) return "LIPS";
// غيّر 45.0 إلى 40.0 مثلاً لجعل المنطقة أوسع
5.3. تغيير سرعة الباود (Baud Rate)
في دالة setup()، يمكنك تغيير قيمة 115200 إلى أي سرعة أخرى:

cpp
Serial.begin(115200); // غيّر إلى 9600 أو 57600
5.4. ضبط معاملات فلتر كالمان
معاملات Q (ضوضاء العملية) و R (ضوضاء القياس) تؤثر على سلاسة الإحداثيات:

cpp
const double Q = 0.022;  // زيادة Q تجعل الفلتر أسرع في الاستجابة
const double R = 0.618;  // زيادة R تجعل الفلتر أكثر سلاسة
5.5. تفعيل/تعطيل وضع التصحيح (Debug Mode)
وضع التصحيح يرسل البيانات الخام والمفلترة عبر المنفذ التسلسلي. يمكنك تعطيله عن طريق التعليق على السطور التالية:

cpp
Serial.print("Raw: "); Serial.print(rawX, 1); // علّق على هذه السطور
6. تطوير الإصدار v4.0 (v4.0 Development)
الكود التجريبي compute_position_v4.cpp يحتوي على نظام حل 4×4 (مع الميكروفون الخامس) لتحسين دقة المحور Z.

6.1. نظام 4×4
cpp
bool computePositionV4(const Point3D mics[4], const double deltaR[4], Point3D& target) {
    // 1. بناء مصفوفة النظام A والمتجه B
    // 2. حل النظام باستخدام حذف غاوسي (Gaussian Elimination)
    // 3. استخراج الإحداثيات مع حماية المحور Z
}6.2. تحسينات مقترحة للتجربة
إضافة فلتر كالمان خفيف: لتنعيم الإحداثيات الخارجة من نظام 4×4.

تحسين دقة التكميم (Sub-sample Interpolation): لرفع دقة الفروق الزمنية عند 16 كيلوهرتز.

7. المساهمة في المشروع (Contributing)
نرحب بمساهماتكم! للمشاركة:

اقرأ دليل المساهمة.

استعرض المشكلات المفتوحة (Issues).

اتبع إجراءات طلب السحب (Pull Request) الموضحة في الدليل.

8. المراجع والدعم (References & Support)
المستودع الرئيسي: GitHub - slam-prog/KineticVoice

التوثيق التقني: TECHNICAL_EVALUATION.md

دليل المستخدم: USER_MANUAL.md

المناقشات: GitHub Discussions

9. الترخيص (License)
هذا الدليل والمشروع مرخصان بموجب الترخيص الإنساني والأخلاقي (HEUL) v1.0. يُرجى قراءة ملف LICENSE لفهم حقوقك والتزاماتك.

شكراً لمساهمتك في تطوير KineticVoice!
"التكنولوجيا يجب أن تكون شجرة طيبة، يأكل من ثمارها الجميع."

