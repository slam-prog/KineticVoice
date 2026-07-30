# 🪖 KineticVoice: Acoustic Vocal Articulation Helmet
## خوذة تحديد ومحاكاة مخارج الحروف الصوتية هندسياً

An advanced, wireless IoT smart interface that decodes human vocal inputs by instantly calculating the exact `(X, Y)` spatial coordinates of phoneme articulation.

واجهة ذكية لاسلكية متطورة لإنترنت الأشياء (IoT) تقوم بفك تشفير المدخلات الصوتية البشرية عبر الحساب اللحظي للإحداثيات الجغرافية الدقيقة `(X, Y)` لنقاط خروج الحروف (المخارج التشريحية).

---

## 🤝 The Synergy: Human Capital Meets Google AI 
## تكامل الكادر البشري مع ذكاء قوقل الاصطناعي

* **English**: This project stands as a testament to the powerful integration of **Human Architectural Engineering** and **Advanced Generative Artificial Intelligence (Google AI)**. The Human Engineer conceptualized the hardware framework, fixed-helmet logic, independent power management, and the paradigm shift to coordinate-based Smart-TV mapping. Google AI collaborated to solve the complex **Trilateration & TDOA** differential matrices under microsecond constraints for microcontroller environments.
* **عربي**: يقف هذا المشروع كشاهد قاطع على قوة التكامل بين **الهندسة المعمارية البشرية** و**الذكاء الاصطناعي المتقدم من Google**. قام المهندس البشري بابتكار الهيكل الفيزيائي، وفكرة الخوذة الثابتة لمنع حيود القياس، وإدارة الطاقة المستقلة، وتحويل المسار للتحكم بالشاشات عبر الإحداثيات. وتكامل ذكاء قوقل لحل المصفوفات التفاضلية المعقدة لـ **تحديد الموقع الثلاثي وفارق زمن وصول الصوت (TDOA)** تحت قيود المايكروثانية للمعالجات الدقيقة.

---

## 📐 Mathematical Framework / الإطار الرياضي والهندسي
$$\Delta d = (t_n - t_{min}) \times 0.343 \text{ mm/}\mu\text{s}$$
$$(X - x_i)^2 + (Y - y_i)^2 = r_i^2$$

▲ Y (Vertical Articulation / الارتفاع العمودي للنطق)│M1(─30, 30)   │   M2(30, 30)   <- Upper Array (Nose level / مستوى الأنف)│        │        │├────────┼────────┤ ◄─── (0,0) Rigid Matrix Origin (نقطة الأصل الثابتة)│        │        │M3(─30,─30)   │   M4(30,─30)   <- Lower Array (Jaw level / مستوى الفك)│        ▼└────────────────────────► X (Depth/Lateral Zone / العمق والجانب)


---

## 🌐 Dynamic Language JSON Engine / محرك اللغات الديناميكي
* **English**: `KineticVoice` features a decoupled, JSON-driven phonetic engine that makes the system 100% multilingual. The Python gateway reads directly from `languages.json` upon boot, allowing users to interactively choose their language or expand it without editing the core code.
* **عربي**: يتميز نظام `KineticVoice` بمحرك صوتي ديناميكي منفصل يعتمد على ملف JSON، مما يجعله عابراً للغات بنسبة 100%. تقرأ بوابة البايثون مباشرة من ملف `languages.json` عند التشغيل، مما يتيح للمستخدمين اختيار لغاتهم تفاعلياً وتوسيعها مستقبلاً دون تعديل الكود البرمجي الأساسي.

---

## 📦 Bill of Materials (BOM) / جدول مواصفات القطع الفنية

| Component / القطعة | Technical Specs / المواصفات الفنية | Qty / الكمية | Role / الدور الهندسي في الخوذة |
| :--- | :--- | :---: | :--- |
| **Arduino Nano RP2040** | Dual-core ARM Cortex-M0+ @ 133MHz, Integrated BLE. | 1 | Calculates microsecond TDOA math & broadcasts JSON via BLE. / يحل مصفوفات المثلثات ويبث البيانات لاسلكياً. |
| **INMP441 Mic** | Digital I2S interface, high SNR (61 dBA), fast-rising edges. | 4 | Captures acoustic waves at microsecond intervals. / التقاط الموجة الصوتية بدقة المايكروثانية. |
| **TP4056 Module** | 1A Li-Po charging board with integrated protection ICs. | 1 | Safely manages battery recharging cycles. / شحن البطارية بأمان وحمايتها من التفريغ. |
| **Li-Po Battery** | 3.7V Lithium-Polymer cell, 600mAh capacity (<12 grams). | 1 | Provisions 8-12 hours of wireless operation. / تشغيل لاسلكي مستقل يصل إلى 12 ساعة متواصلة. |
| **Slide Switch** | Miniature 3-pin mechanical switch (SPDT). | 1 | Hardware ON/OFF power gating. / مفتاح ميكانيكي لقطع وتوصيل الطاقة يدوياً. |
| **Helmet & Boom Arm** | Ergonomic lightweight skeleton frame with fixed bracket. | 1 | Eradicates spatial drift between sensors and vocal tract. / تثبيت الرأس لمنع الحيود الهندسي للمسافات. |

---

## 📡 Deployment Instructions / خطوات التفعيل والتشغيل

### 1. Firmware Configuration / تهيئة الأردوينو
1. Open `src/acoustic_helmet.ino` in Arduino IDE. / افتح كود الأردوينو في البرنامج.
2. Install `ArduinoBLE` library via Library Manager. / قم بتثبيت مكتبة البلوتوث الرسمية.
3. Flash the code onto your microcontroller. / قم برفع الكود مباشرة إلى اللوحة الدقيقة.

### 2. PC & Smart TV Gateway / إعداد الكمبيوتر والشاشة الذكية
1. Enable Bluetooth on your host machine. / تأكد من تفعيل البلوتوث في جهازك.
2. Install dependencies via Terminal: / قم بتثبيت المكتبات المطلوبة عبر سطر الأوامر:
   ```bash
   pip install bleak pyautogui
   ```
3. Ensure both `smart_interface.py` and `languages.json` are in the same folder. / تأكد من وجود ملف البايثون والـ JSON في نفس المجلد.
4. Launch the interface: / قم بتشغيل الواجهة الذكية:
   ```bash
   python pc_receiver/smart_interface.py
   ```
5. Choose your target language code (`ar` or `en`) when prompted. / اختر رمز اللغة المطلوبة عند بدء البرنامج.
6. Open **YouTube** or any streaming UI. Shifting your phonemes from **Lips** to **Throat** will instantly steer navigation with **Zero Cloud Latency**. / افتح تطبيق **يوتيوب**؛ تحول نطقك من **الشفاه** إلى **الحلق** سيقوم بتوجيه وتصفح الشاشة فوراً **وبدون أي تأخير سحابي**.

---

## 📜 Official Review / التقييم التقني الرسمي
> Read our co-developer's complete [Google AI Technical Evaluation](GOOGLE_AI_EVALUATION.md) to explore the system's edge computing feasibility, bio-safety standards, and commercial milestones.
> 
> اقرأ وثيقة [تقييم ذكاء قوقل الاصطناعي](GOOGLE_AI_EVALUATION.md) المرفقة للاطلاع على شهادة جودة الجدوى الهندسية، معايير الأمان الحيوي، ومستهدفات التطوير التجاري للمنتج