#!/usr/bin/env python3
"""
مثال للتكامل مع DeepSeek API
قراءة البيانات من KineticVoice عبر BLE وتحليلها باستخدام DeepSeek
"""

import asyncio
import re
from bleak import BleakScanner, BleakClient
from openai import OpenAI

# ===== الإعدادات =====
DEVICE_NAME = "KineticVoiceCore"
CHARACTERISTIC_UUID = "2A19"
DEEPSEEK_API_KEY = "YOUR_DEEPSEEK_API_KEY"  # استبدل بمفتاحك

# تهيئة DeepSeek Client
client = OpenAI(
    api_key=DEEPSEEK_API_KEY,
    base_url="https://api.deepseek.com"
)

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

def analyze_with_deepseek(kinetic_data: dict) -> str:
    """إرسال البيانات إلى DeepSeek API وتحليلها"""
    prompt = f"""
    You are a speech analysis assistant. Given the following articulatory data:
    - X: {kinetic_data['x']} mm (lateral position)
    - Y: {kinetic_data['y']} mm (vertical position)
    - Z: {kinetic_data['z']} mm (depth)
    - Zone: {kinetic_data['zone']} (anatomical zone)

    What can you infer about the speaker's pronunciation? Which phoneme does this likely represent?
    Provide a brief explanation.
    """
    
    response = client.chat.completions.create(
        model="deepseek-v4-pro",  # أو deepseek-v4-flash
        messages=[
            {"role": "system", "content": "You are a helpful speech analysis assistant."},
            {"role": "user", "content": prompt}
        ],
        reasoning_effort="high"
    )
    return response.choices[0].message.content

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
                    print(f"🧠 Analyzing with DeepSeek...")
                    result = analyze_with_deepseek(parsed)
                    print(f"💬 DeepSeek Analysis:\n{result}\n")
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
