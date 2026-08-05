# أمثلة التكامل مع KineticVoice
## Integration Examples for KineticVoice

هذا المجلد يحتوي على أمثلة عملية لاستخدام بيانات منصة KineticVoice مع خدمات الذكاء الاصطناعي المختلفة.

### المتطلبات الأساسية (Prerequisites)

- **Python 3.8+** (لتشغيل أمثلة Python).
- **مكتبات Python** (يمكن تثبيتها عبر `requirements.txt`):
  - `bleak` (للاتصال بـ BLE)
  - `google-generativeai` (لـ Gemini API)
  - `openai` (لـ DeepSeek API)
- **مفتاح API** من Google AI Studio أو DeepSeek Platform.
- **جهاز KineticVoice** قيد التشغيل ويبث البيانات.

### محتويات المجلد

| الملف | الوصف |
| :--- | :--- |
| `gemini_analysis.py` | مثال لقراءة البيانات من الجهاز وتحليلها باستخدام Google Gemini API. |
| `deepseek_analysis.py` | مثال لقراءة البيانات وتحليلها باستخدام DeepSeek API. |
| `web_ble_dashboard.html` | واجهة ويب بسيطة (JavaScript) لقراءة البيانات وعرضها في الوقت الفعلي. |

### كيفية التشغيل

1. **تثبيت المكتبات**:
   ```bash
   pip install -r requirements.txt
تشغيل مثال Python:

bash
python gemini_analysis.py
أو:

bash
python deepseek_analysis.py
تشغيل واجهة الويب:

افتح ملف web_ble_dashboard.html في متصفح حديث (يدعم Web Bluetooth).

اضغط على زر "Connect" واختر جهاز KineticVoiceCore.
