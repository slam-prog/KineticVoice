import asyncio
import json
import time
import pyautogui # لمحاكاة الأزرار على أنظمة التلفاز والكمبيوتر واليوتيوب
from bleak import BleakClient, BleakScanner

DEVICE_NAME = "AcousticHelmet"
CHARACTERISTIC_UUID = "2A19"
TEXT_LOG_FILE = "transcribed_output.txt"

def parse_and_execute(zone):
    """
    تحويل مخارج الأحرف والاهتزاز النطقي إلى أوامر أجهزة ريموت ذكية وحفظها نصياً.
    """
    zones_text = {
        "LABIAL_LIPS": "[ب، م، و، ف] ",
        "DENTAL_ALVEOLAR": "[د، ت، ن، ط، ز، س] ",
        "PALATAL_MID_MOUTH": "[ج، ش، ي، ك] ",
        "PHARYNGEAL_THROAT": "[ع، ح، خ، غ، أ، هـ] "
    }
    
    # 1. كتابة الحرف المكتشف في الملف النصي
    char_discovered = zones_text.get(zone, "[؟] ")
    with open(TEXT_LOG_FILE, "a", encoding="utf-8") as f:
        f.write(char_discovered)
        f.flush()

    # 2. ترجمة المخرج إلى أمر ملاحة ميكانيكي (Smart Remote Action)
    if zone == "LABIAL_LIPS":
        pyautogui.press('enter') # الشفاه = اختيار / تشغيل (Play/Pause)
        return "ENTER / PLAY"
    elif zone == "DENTAL_ALVEOLAR":
        pyautogui.press('down')  # اللسان = تمرير لأسفل وتصفح فيديوهات يوتيوب
        return "SCROLL DOWN"
    elif zone == "PALATAL_MID_MOUTH":
        pyautogui.press('up')    # وسط الفم = تمرير لأعلى
        return "SCROLL UP"
    elif zone == "PHARYNGEAL_THROAT":
        pyautogui.press('backspace') # الحلق = زر العودة للخلف (Back)
        return "GO BACK"
    return "NO_ACTION"

def notification_handler(sender, data):
    try:
        raw_data = data.decode('utf-8').strip()
        if raw_data == "{}": return
        
        payload = json.loads(raw_data)
        x, y, zone = payload.get("X"), payload.get("Y"), payload.get("ZONE")
        bat = payload.get("BATTERY", 100)
        
        # معالجة وتنفيذ الأمر وحفظ النص
        action = parse_and_execute(zone)
        
        print(f"[Helmet BLE] X: {x}mm, Y: {y}mm | Zone: {zone} | Battery: {bat}% | Command: {action}")
    except Exception as e:
        pass

async def main():
    print("[*] Scanning for Hardware-Driven Acoustic Helmet (BLE)...")
    devices = await BleakScanner.discover()
    target_address = None
    
    for d in devices:
        if d.name == DEVICE_NAME:
            target_address = d.address
            break
            
    if not target_address:
        print("[-] Helmet Interface not found. Check power supply.")
        return
        
    async with BleakClient(target_address) as client:
        print("[+] Universal Smart Interface Synced! Control your TV/Apps now via Speech Articulation.")
        with open(TEXT_LOG_FILE, "a", encoding="utf-8") as f:
            f.write(f"\n--- Session Opened: {time.strftime('%Y-%m-%d %H:%M:%S')} ---\n")
            
        await client.start_notify(CHARACTERISTIC_UUID, notification_handler)
        while True:
            await asyncio.sleep(1)

if __name__ == "__main__":
    asyncio.run(main())
