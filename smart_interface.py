import asyncio
import json
import os
import time
import pyautogui  # لمحاكاة الأزرار على أنظمة التلفاز والكمبيوتر واليوتيوب
from bleak import BleakClient, BleakScanner

DEVICE_NAME = "AcousticHelmet"
CHARACTERISTIC_UUID = "2A19"
TEXT_LOG_FILE = "transcribed_output.txt"
LANG_FILE = "languages.json"

# متغيرات عالمية لحفظ إعدادات اللغة المختارة
active_language_map = {}
selected_lang_code = "ar"

def load_language_interface():
    """تحميل اللغات المتاحة من ملف JSON والسماح للمستخدم بالاختيار يدوياً"""
    global active_language_map, selected_lang_code
    
    if not os.path.exists(LANG_FILE):
        print(f"[-] Error: '{LANG_FILE}' not found! Creating a default one...")
        # إنشاء ملف افتراضي في حال عدم وجوده
        default_data = {
            "ar": {"language_name": "العربية", "LABIAL_LIPS": "[ب، م، و، ف] ", "DENTAL_ALVEOLAR": "[د، ت، ن، ط، ز، س] ", "PALATAL_MID_MOUTH": "[ج، ش، ي، ك] ", "PHARYNGEAL_THROAT": "[ع، ح، خ، غ، أ، هـ] "},
            "en": {"language_name": "English", "LABIAL_LIPS": "[B, M, W, P, F] ", "DENTAL_ALVEOLAR": "[D, T, N, S, Z, L] ", "PALATAL_MID_MOUTH": "[J, SH, Y, CH, K] ", "PHARYNGEAL_THROAT": "[H, G, O, A] "}
        }
        with open(LANG_FILE, "w", encoding="utf-8") as f:
            json.dump(default_data, f, ensure_ascii=False, indent=2)
            
    with open(LANG_FILE, "r", encoding="utf-8") as f:
        all_languages = json.load(f)
        
    print("=== KineticVoice Language Selection / اختيار اللغة ===")
    for code, info in all_languages.items():
        print(f"[{code}] - {info['language_name']}")
        
    choice = input("\nEnter language code (e.g., ar / en): ").strip().lower()
    if choice in all_languages:
        selected_lang_code = choice
        active_language_map = all_languages[choice]
        print(f"[+] Active Language Set to: {active_language_map['language_name']}\n")
    else:
        selected_lang_code = "ar"
        active_language_map = all_languages["ar"]
        print("[!] Invalid choice. Defaulting to: العربية\n")

def parse_and_execute(zone):
    """
    ربط الإحداثيات بالخرائط الصوتية المستدعاة ديناميكياً من الـ JSON 
    وتنفيذ الأوامر الميكانيكية للريموت الذكي.
    """
    global active_language_map
    
    # استخراج الحروف المقابلة للمخرج بناءً على اللغة المحددة من ملف JSON
    char_discovered = active_language_map.get(zone, "[؟] ")
    
    # 1. تدوين الحروف المكتشفة في الملف النصي التاريخي للجلسة
    with open(TEXT_LOG_FILE, "a", encoding="utf-8") as f:
        f.write(char_discovered + " ")
        f.flush()

    # 2. محاكاة أزرار التحكم بالتلفاز الذكي واليوتيوب (HID Injection)
    if zone == "LABIAL_LIPS":
        pyautogui.press('enter')       # الشفاه = تأكيد وتشغيل الفيديو
        return f"ENTER / PLAY -> Chars: {char_discovered}"
    elif zone == "DENTAL_ALVEOLAR":
        pyautogui.press('down')        # طرف اللسان = التمرير لأسفل القائمة
        return f"SCROLL DOWN -> Chars: {char_discovered}"
    elif zone == "PALATAL_MID_MOUTH":
        pyautogui.press('up')          # وسط الفم = التمرير لأعلى القائمة
        return f"SCROLL UP -> Chars: {char_discovered}"
    elif zone == "PHARYNGEAL_THROAT":
        pyautogui.press('backspace')   # الحلق = زر الرجوع للخلف
        return f"GO BACK -> Chars: {char_discovered}"
        
    return "NO_ACTION"

def notification_handler(sender, data):
    try:
        raw_data = data.decode('utf-8').strip()
        if raw_data == "{}": return
        
        payload = json.loads(raw_data)
        x, y, zone = payload.get("X"), payload.get("Y"), payload.get("ZONE")
        bat = payload.get("BATTERY", 100)
        
        # معالجة الحركة الصوتية، تدوين الحرف، وتنفيذ الإجراء الذكي
        action = parse_and_execute(zone)
        
        print(f"[KineticVoice BLE] Coords: ({x}mm, {y}mm) | Battery: {bat}% | Action: {action}")
    except Exception as e:
        pass

async def main():
    # تحميل واختيار اللغات ديناميكياً قبل بدء الاتصال اللاسلكي
    load_language_interface()
    
    print("[*] Scanning for Hardware-Driven Acoustic Helmet (BLE)...")
    devices = await BleakScanner.discover()
    target_address = None
    
    for d in devices:
        if d.name == DEVICE_NAME:
            target_address = d.address
            break
            
    if not target_address:
        print("[-] Helmet Interface not found. Please verify hardware power.")
        return
        
    async with BleakClient(target_address) as client:
        print(f"[+] Connected to KineticVoice Helmet! Session Language: [{selected_lang_code.upper()}]")
        with open(TEXT_LOG_FILE, "a", encoding="utf-8") as f:
            f.write(f"\n--- Wireless Session ({selected_lang_code.upper()}): {time.strftime('%Y-%m-%d %H:%M:%S')} ---\n")
            
        await client.start_notify(CHARACTERISTIC_UUID, notification_handler)
        while True:
            await asyncio.sleep(1)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n[-] Dynamic Session safely closed by user.")
