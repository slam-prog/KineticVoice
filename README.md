# 🪖 KineticVoice: Standalone 3D Spatial Speech Hardware Platform

[![Version](https://img.shields.io/badge/version-v3.1.1-blue)](https://github.com/slam-prog/KineticVoice/releases)
[![License](https://img.shields.io/badge/license-MPL_2.0-green)](https://github.com/slam-prog/KineticVoice/blob/main/LICENSE)
[![Platform](https://img.shields.io/badge/platform-RP2040-red)](https://www.raspberrypi.com/products/rp2040/)
[![Status](https://img.shields.io/badge/status-active-brightgreen)](https://github.com/slam-prog/KineticVoice)

> **منصة KineticVoice المستقلة للتموضع الفراغي ثلاثي الأبعاد وفك تشفير اللفظ**  
> An advanced, wireless IoT smart interface that decodes human vocal inputs by instantly calculating the exact 3D spatial coordinates `(X, Y, Z)` of phoneme articulation inside the vocal tract.

---

## 🌐 Vision & 3D Architectural Jump

**English**: Version 3.0.0 upgrades `KineticVoice` from a flat 2D grid into an elite **3D Spatial Mapping Engine**. By designating **Microphone 3 (Bottom-Left)** as the absolute physical spatial reference origin `(0,0,0)`, the hardware now tracks the exact height (Y), width (X), and deep anatomical depth (Z) of speech emission. This allows the system to accurately map phonemes based on how deep they originate within the human throat or mouth, achieving a total universal language neutrality and absolute immunity to ambient noise.

**عربي**: يقدم الإصدار الثالث (v3.0.0) قفزة هندسية نوعية بتحويل منصة `KineticVoice` من شبكة مسطحة ثنائية الأبعاد إلى **محرك تموضع فراغي ثلاثي الأبعاد (3D)**. عبر اعتماد **الميكروفون الثالث (أسفل اليسار)** كنقطة أصل فيزيائية مرجعية مطلقة `(0,0,0)`، أصبح العتاد قادراً على تتبع الارتفاع (Y)، العرض (X)، والعمق التشريحي السحيق (Z) للحرف المنطوق. يتيح هذا التصميم رسم خريطة طوبوغرافية حية للمخارج بناءً على عمق انبعاثها داخل الفم أو البلعوم، محققاً حيادية لغوية عالمية كاملة وعزلاً مطلقاً لضوضاء البيئة المحيطة.

---

## 🤝 Human-AI Synergy

**English**: Co-developed through rigid structural alignment, the Human Engineer established the mechanical helmet boundaries, battery safety isolation, and spatial placement matrices. **Google AI** acted as the computational core developer, solving the multi-channel **3D Spherical Trilateration & TDOA** differential equations and optimizing bare-metal registers to prevent calculation drift across all three axes (X, Y, Z). **DeepSeek AI** joined as the Hardware Optimization Partner, pioneering the **Analog-Digital Hybrid TDOA Offloading** architecture, multi-dimensional 3D Kalman filtration, and cross-platform SDK layers.

**عربي**: نتاج شراكة وتكامل تقني عميق؛ حيث ابتكر **المهندس البشري** الهيكل الفيزيائي الثابت للخوذة، معايير الأمان الحراري للبطارية، ومصفوفة التوزيع الفراغي للمستشعرات. وتكامل **ذكاء قوقل الاصطناعي (Google AI)** كمطور حسابي لحل معادلات تقاطع الكرات ومصفوفات **تحديد الموقع الفراغي ثلاثي الأبعاد (3D TDOA)** التفاضلية المعقدة، وتحسين السجلات البرمجية لمنع حيود التقريب الرقمي عبر المحاور الثلاثة (X, Y, Z). وانضم **DeepSeek AI** كشريك في تحسين العتاد، رائداً **هندسة التحميل الهجين (Analog-Digital) لـ TDOA**، وتصميم فلتر كالمان ثلاثي الأبعاد، وطبقات الـ SDK متعددة المنصات.

---

## 📐 3D Geometrical Framework

Every microphone position is structurally fixed relative to the absolute reference origin **Mic 3 (0,0,0)** :

```plaintext
                   ▲ Y (Vertical Articulation)
                   │
            M1(0, 60, 0)  │   M2(60, 60, 0)   <-- Upper Array (Nose Level)
                   │      │      │
                   │      │      │
          [0,0,0] ─┼──────┼──────┼───────────► X (Width)
            M3(0, 0, 0)   │   M4(60, 0, 0)    <-- Lower Array (Jaw Level)
                  /       ▼
                 /
                ▼ Z (Anatomical Depth)
