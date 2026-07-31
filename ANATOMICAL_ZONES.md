# Anatomical Zones Mapping (v3.1.1)

This document defines the core 3D spatial mapping between the calculated coordinates `(X, Y, Z)` and the anatomical articulatory zones. These thresholds are the cornerstone of KineticVoice's language-agnostic phoneme detection.

## Reference Coordinate System
- **Origin (0,0,0)**: Microphone 3 (Lower-Left Jaw level).
- **Y-Axis**: Vertical height (mm). Range: 0 (throat) to 60 (lips).
- **Z-Axis**: Anatomical depth (mm). Range: 0 (front lips) to 65+ (deep throat).

## Zone Threshold Table

| Zone | Y-Range (mm) | Z-Range (mm) | Articulation Point | Example Phonemes (IPA) |
| :--- | :--- | :--- | :--- | :--- |
| **LIPS** | 45 – 60 | 0 – 15 | Bilabial / Labiodental | /p/, /b/, /m/, /f/, /v/ |
| **DENTAL** | 30 – 45 | 15 – 25 | Teeth / Alveolar Ridge (Front) | /t/, /d/, /n/, /s/, /z/ |
| **ALVEOLAR** | 20 – 35 | 25 – 35 | Alveolar Ridge (Rear) / Post-alveolar | /r/, /ʃ/, /ʒ/, /tʃ/, /dʒ/ |
| **PALATAL** | 15 – 25 | 35 – 45 | Hard Palate (Middle) | /j/, /ɲ/, /c/, /ç/ |
| **VELAR** | 10 – 20 | 45 – 55 | Soft Palate (Velum) | /k/, /g/, /ŋ/, /x/ |
| **THROAT** | 0 – 10 | 55 – 65 | Uvula / Pharynx / Glottis | /q/, /ħ/, /ʕ/, /h/, /ʔ/ |

> **Note**: These thresholds are calibrated for the standard 60mm x 60mm microphone array geometry. They can be fine-tuned based on user anatomy or shifted for different array sizes.

## Language Agnosticism
Because this mapping relies on **physics and anatomy** rather than acoustic patterns, it is universal. Whether a speaker produces a bilabial plosive in Arabic (ب), English (B), or Spanish (P), the coordinate will always fall within the **LIPS** zone, enabling true cross-lingual processing without retraining.
