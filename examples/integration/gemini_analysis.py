#!/usr/bin/env python3
"""
مثال للتكامل مع Google Gemini API
قراءة البيانات من KineticVoice عبر BLE وتحليلها باستخدام Gemini
"""

import asyncio
import re
from bleak import BleakScanner, BleakClient
import google.generativeai as genai

# ===== الإعدادات =====
DEVICE_NAME = "KineticVoiceCore"
CHARACTERISTIC_UUID = "2A19"
GEMINI_API_KEY = "YOUR_GEMINI_API_KEY"  # استبدل بمفتاحك

# تهيئة Gemini
genai.configure(api_key=GEMINI_API_KEY)
model = genai.GenerativeModel('gemini-1.5-flash')

def parse_kinetic_data(data_str: str) -> dict:
    """تحويل البيانات النصية إلى قاموس"""
    try:
        parts = data_str.strip().split(',')
        return {
            'x': float(parts[0]),
            'y': float(parts[1]),
            'z': float(parts[2]),
            'zone': parts[3]
        }
    except:
        return None

def analyze_with_gemini(kinetic_data: dict) -> str:
    """إرسال البيانات إلى Gemini وتحليلها"""
    prompt = f"""
    You are a speech analysis assistant. Given the following articulatory data from a speaker:
    - X: {kinetic_data['x']} mm (lateral position)
    - Y: {kinetic_data['y']} mm (vertical position)
    - Z: {kinetic_data['z']} mm (depth)
    - Zone: {kinetic_data['zone']} (anatomical zone)

    What can you infer about the speaker's pronunciation? Which phoneme does this likely represent?
    Provide a brief explanation in plain English.
    """
    response = model.generate_content(prompt)
    return response.text

async def main():
    print("🔍 Scanning for KineticVoice device...")
    device = await BleakScanner.find_device_by_name(DEVICE_NAME)
    if not device:
        print("❌ Device not found. Make sure it's powered on and advertising.")
        return

    print(f"✅ Device found: {device.name} ({device.address})")
    async with BleakClient(device) as client:
        print("🔗 Connected!")

        def notification_handler(sender, data):
            try:
                data_str = data.decode('utf-8').strip()
                print(f"\n📊 Raw Data: {data_str}")
                parsed = parse_kinetic_data(data_str)
                if parsed:
                    print(f"🧠 Analyzing with Gemini...")
                    result = analyze_with_gemini(parsed)
                    print(f"💬 Gemini Analysis:\n{result}\n")
            except Exception as e:
                print(f"⚠️ Error processing data: {e}")

        await client.start_notify(CHARACTERISTIC_UUID, notification_handler)
        print("👂 Listening for data... Press Ctrl+C to stop.")
        try:
            await asyncio.Event().wait()
        except KeyboardInterrupt:
            print("\n👋 Disconnecting...")
            await client.stop_notify(CHARACTERISTIC_UUID)

if __name__ == "__main__":
    asyncio.run(main())
