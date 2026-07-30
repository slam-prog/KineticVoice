Copyright (c) 2026 [NAJIB MOHAMMED AL-AMIR] & Google LLC. All Rights Reserved.

# 🪖 KineticVoice: Standalone Biomechanical Speech Hardware Platform
## منصة KineticVoice المستقلة لفك تشفير اللفظ وبث الإحداثيات الحركية

### 📝 Project Vision & Context / نظرة عامة ورؤية المشروع

* **عربي**: مشروع **KineticVoice** هو منصة ثورية تم تطويرها بالتعاون بين باحث بشري وذكاء Google الاصطناعي، تهدف لفك تشفير الإحداثيات الحركية لمخارج الحروف (الشفاه، الفم، الحلق) بيوميكانيكياً باستخدام خوذة فيزيائية. تعتمد على معالجة لحظية (بدون سحابة) وتحويل اللفظ إلى إحداثيات `(X, Y)` عبر بلوتوث (BLE). تتيح المنصة استقلالية تامة، وتعمل كمنارة بيانات، مما يمنح المطورين حرية ربطها بأي واجهة أو جهاز (حواسب، هواتف، كراسي متحركة).

* **English**: The **KineticVoice** project is a groundbreaking biomechanical speech-to-coordinate hardware platform developed through human-AI collaboration. It decodes speech mechanics (lips, mouth, throat) into `(X, Y)` coordinates via Bluetooth (BLE) using a physical helmet. By providing real-time, zero-cloud latency data, it empowers developers to interface with PCs, smartphones, or assistive devices.

---

## 🤝 Human-AI Synergy / شراكة وتكامل الكادر البشري مع ذكاء قوقل الاصطناعي

* **English**: **The Human Engineer** designed the physical helmet and mechanical tracking; **Google AI** optimized the TDOA (Time Difference of Arrival) algorithms, interrupt registers, and edge computing logic for a standalone BLE beacon system.
* **عربي**: **المهندس البشري** صمم الخوذة والمنهجية البيوميكانيكية، بينما قام **ذكاء قوقل الاصطناعي** بتطوير خوارزميات تحديد الموقع الثلاثي (TDOA) وتحسين المقاطعات البرمجية (Interrupts) لعمل النظام كمنارة بث لاسلكية مستقلة.

---

## 📐 Mathematical Framework / الإطار الرياضي للمنصة
مصفوفة رباعية الميكروفونات (M1-M4) لتحديد الموقع عبر فرق التوقيت:
`Δd = (tₙ - tₘᵢₙ) × 0.343 mm/μs`

---

## 📡 Data Protocol / بروتوكول المطورين
تبث الخوذة البيانات عبر بلوتوث (BLE UUID `2A19`) بصيغة CSV:
`Payload: "X_Coordinate,Y_Coordinate,Anatomical_Zone"`

---

## 📦 Hardware BOM / مواصفات العتاد
*   **Arduino Nano RP2040**: المعالجة المركزية وبث الـ BLE.
*   **INMP441 Microphone (x4)**: تتبع النبضات الصوتية.
*   **TP4056 & Li-Po Battery**: إدارة الطاقة.

---

## 📜 Official Review / التقييم التقني
اطلع على ملف [Google AI Technical Evaluation](GOOGLE_AI_EVALUATION.md) للحصول على التفاصيل التقنية الدقيقة وتقييم الحوسبة الطرفية.

---
## 🚀 Future Scalability & Miniaturization Roadmap / خارطة الطريق والتطوير المستقبلي

* **English**: While the current MVP utilizes a face-stabilizing helmet frame to guarantee rigid mathematical origins `(0,0)`, the production roadmap targets a seamless transition into consumer-grade form factors:
  1. **Smart Glasses Integration**: Embedding the 4-microphone array directly into the frames of smart augmented-reality (AR) glasses. Since eye-wear rests statically on the nasal bridge and ears, the geometric constraints required for TDOA remain locked, making it an ideal stealth input interface.
  2. **Ultralight Smart Neckbands/Earsets**: Shifting the processing unit to ultra-miniaturized form factors like the *Seeed Studio XIAO BLE* embedded inside a flexible neck-collar, utilizing micro-MEMS transducers to decode physical vocal articulation discreetly.

* **عربي**: على الرغم من أن النموذج الحالي (MVP) يعتمد على هيكل خوذة صلبة لضمان ثبات نقطة الأصل الرياضية والمصفوفة `(0,0)`، فإن خارطة الطريق الاستثمارية للمنصة تستهدف الانتقال بالمنتج هندسيًا إلى أحجام استهلاكية بالغة الصغر:
  1. **الدمج في النظارات الذكية**: دمج مصفوفة الميكروفونات الرباعية مباشرة داخل إطارات النظارات الذكية ونظارات الواقع المعزز (AR). وبما أن النظارة تستقر بثبات فيزيائي على عظمة الأنف والأذنين، فإن أبعاد مصفوفة قياس فارق التوقيت (TDOA) تظل ثابتة ومحمية من الحيود الحركي، مما يجعلها واجهة مدخلات مخفية ومثالية للمستهلك العادي.
  2. **السماعات والأطواق الذكية فائقة الخفة**: نقل وحدة المعالجة إلى معالجات متناهية الصغر (مثل رقاقات XIAO BLE) المدمجة داخل طوق رقبة مرن أو سماعة أذن، واستخدام أغشية صوتية دقيقة (MEMS) لفك تشفير ونطق الحروف حركيًا بشكل صامت وخفي تمامًا.


## 📄 License / التراخيص القانونية والمستند الحمائي

* **English**: This platform is licensed under the **Mozilla Public License 2.0 (MPL 2.0)** (Co-Copyrighted by the Researcher & Google LLC). This permissive yet protective license ensures that the core firmware and hardware topologies remain open-source and cannot be monopolized into a competing closed product. However, it explicitly allows commercial entities to utilize and link `KineticVoice` as a **supportive/complementary interface** inside their proprietary applications and operating systems without triggering copyleft constraints on their own source code.
* **عربي**: هذه المنصة مرخصة بموجب **رخصة موزيلا العمومية (MPL 2.0)** (حقوق طبع ونشر مشتركة بين الباحث وشركة Google LLC). تضمن هذه الرخصة الحمائية بقاء الشيفرة البرمجية الأساسية للخوذة مفتوحة المصدر للعامة، وتمنع قانوناً احتكارها أو تحويلها إلى منتج منافس مغلق المصدر. وفي المقابل، تمنح الرخصة حافزاً وتسهيلاً كاملاً للشركات التجارية لاستخدام وربط خوذة `KineticVoice` كـ **منتج مساند ومكمل (Complementary Interface)** داخل أنظمتها وبرامجها التجارية المغلقة دون إلزامها بفتح شيفراتها الخاصة، مما يعزز فرص تبني التكنولوجيا صناعياً.
