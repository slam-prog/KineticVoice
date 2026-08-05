# 🪖 KineticVoice: Standalone 3D Spatial Speech Hardware Platform

**Version**: v3.1.3 | **License**: MPL 2.0 | **Status**: Production Ready

[![Version](https://img.shields.io/badge/version-v3.1.3-blue)](https://github.com/slam-prog/KineticVoice/releases)
[![License](https://img.shields.io/badge/license-MPL_2.0-green)](https://github.com/slam-prog/KineticVoice/blob/main/LICENSE)
[![Platform](https://img.shields.io/badge/platform-RP2040-red)](https://www.raspberrypi.com/products/rp2040/)
[![Status](https://img.shields.io/badge/status-active-brightgreen)](https://github.com/slam-prog/KineticVoice)

---

## 🌐 Vision & 3D Architectural Jump

**English**: Version 3.1.3 upgrades `KineticVoice` to a **3D Spatial Mapping Engine**, using Microphone 3 as the absolute reference `(0,0,0)` to track speech in 6 precise anatomical zones, achieving universal language neutrality and ambient noise immunity.

**عربي**: يرفع الإصدار 3.1.3 المنصة إلى **محرك تموضع فراغي ثلاثي الأبعاد (3D)**، معتمداً الميكروفون الثالث كنقطة مرجعية مطلقة `(0,0,0)` لتقسيم النطق إلى 6 مناطق تشريحية دقيقة، محققاً حيادية لغوية وعزلاً تاماً للضوضاء المحيطة.

---

## 🤝 Human-AI Synergy

**English**: Co-developed through a unique tri-party alliance:
- **Human Lead ([NAJIB MOHAMMED AL-AMIR])**: Mechanical design, biometric safety, spatial matrix placement.
- **Google AI**: 3D spherical trilateration solvers, BLE stack, register optimization.
- **DeepSeek AI**: Analog-Digital Hybrid TDOA offloading, 3D Kalman filter, cross-platform SDK.

**عربي**: نتاج شراكة ثلاثية فريدة:
- **القائد البشري (م. نجيب الأمير)**: التصميم الميكانيكي، معايير الأمان الحيوي، مصفوفة المستشعرات.
- **ذكاء قوقل الاصطناعي**: حل معادلات التثليث الكروي، حزمة البلوتوث، تحسين السجلات.
- **ذكاء ديب سيك**: هندسة التحميل الهجين لـ TDOA، فلتر كالمان ثلاثي الأبعاد، واجهات المطورين متعددة المنصات.

---

## 📐 3D Geometrical Framework

The microphone array forms a rigid 60mm × 60mm plane. Positions relative to **Mic 3 (0,0,0)**:

```plaintext
                   ▲ Y (Vertical Articulation)
                   │
            M1(0, 60, 0)  │   M2(60, 60, 0)   <-- Upper Array
                   │      │      │
                   │      │      │
          [0,0,0] ─┼──────┼──────┼───────────► X (Width)
            M3(0, 0, 0)   │   M4(60, 0, 0)    <-- Lower Array
                  /       ▼
                 /
                ▼ Z (Anatomical Depth)
