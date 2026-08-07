import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import least_squares
from mpl_toolkits.mplot3d import Axes3D
import time

# ===================== 1. إعدادات المحاكاة =====================
fs = 16000                   # تردد أخذ العينات (Hz)
duration = 0.5               # مدة الإشارة (ثانية)
c = 343.0                    # سرعة الصوت (m/s)
d = 0.05                     # المسافة بين الميكروفونات (متر)
SNR_dB = 20                  # نسبة الإشارة إلى الضوضاء (ديسيبل)
n_samples = int(fs * duration)
t = np.linspace(0, duration, n_samples)

# ===================== 2. مواقع الميكروفونات =====================
mic_positions = {
    'Mic1': np.array([0.0, 0.0, 0.0]),       # مركزي
    'Mic2': np.array([d, 0.0, 0.0]),         # X-axis
    'Mic3': np.array([0.0, d, 0.0]),         # Y-axis
    'Mic4': np.array([0.0, 0.0, d]),         # Z-axis
    'Mic5': np.array([0.02, 0.0, 0.0])       # مرجعي قريب (close-talk)
}

# ===================== 3. مصدر الصوت والإشارة =====================
source_true = np.array([0.30, 0.20, 0.10])   # الموقع الحقيقي (متر)

# توليد إشارة شبيهة بالكلام (chirp + مكونات عشوائية)
t_chirp = np.linspace(0, duration, n_samples)
signal = np.sin(2 * np.pi * (100 + 500 * t_chirp) * t_chirp) + \
         0.5 * np.random.randn(n_samples)
signal = signal / np.max(np.abs(signal)) * 0.8  # تطبيع

# ===================== 4. محاكاة استقبال الميكروفونات =====================
def simulate_mic_signals(source_pos, mic_positions, signal, fs, c, snr_db=20):
    mic_signals = {}
    for name, mic_pos in mic_positions.items():
        # حساب المسافة وزمن الوصول
        distance = np.linalg.norm(source_pos - mic_pos)
        delay = distance / c
        delay_samples = int(round(delay * fs))
        
        # تطبيق التأخير (مع إزاحة دائرية)
        if delay_samples >= len(signal):
            delayed_signal = np.zeros_like(signal)
        else:
            delayed_signal = np.roll(signal, delay_samples)
        
        # إضافة كسب عشوائي وضوضاء
        gain = np.random.uniform(0.8, 1.2)
        noise_power = np.var(signal) / (10 ** (snr_db / 10))
        noise = np.random.normal(0, np.sqrt(noise_power), len(signal))
        mic_signals[name] = gain * delayed_signal + noise
    
    return mic_signals

mic_signals = simulate_mic_signals(source_true, mic_positions, signal, fs, c, SNR_dB)

# ===================== 5. خوارزمية تنوير لتقدير التأخير =====================
def estimate_delay_tanweer(ref_signal, target_signal, max_shift_samples):
    """
    تقدير التأخير باستخدام خوارزمية تنوير:
    - تحويل الإشارات إلى نسب (تطبيع L2)
    - التدوير الدائري والطرح المباشر
    - اختيار أقل خطأ مطلق لتقدير التأخير
    """
    # تطبيع L2 (تحويل إلى نسب داخلية)
    ref_norm = ref_signal / (np.linalg.norm(ref_signal) + 1e-6)
    target_norm = target_signal / (np.linalg.norm(target_signal) + 1e-6)
    
    errors = []
    shifts = range(-max_shift_samples, max_shift_samples + 1)
    
    for shift in shifts:
        shifted = np.roll(target_norm, shift)
        error = np.sum(np.abs(ref_norm - shifted))
        errors.append(error)
    
    errors = np.array(errors)
    best_shift = shifts[np.argmin(errors)]
    min_error = np.min(errors)
    return best_shift, min_error, shifts, errors

# حساب التأخيرات لكل ميكروفون محيطي باستخدام خوارزمية تنوير
max_shift_samples = int(0.005 * fs)  # ±5 مللي ثانية
ref_signal = mic_signals['Mic5']    # الإشارة المرجعية (الأقرب للفم)

delays_estimated = {}
for name in ['Mic2', 'Mic3', 'Mic4']:
    best_shift, min_err, shifts, errors = estimate_delay_tanweer(
        ref_signal, mic_signals[name], max_shift_samples
    )
    delays_estimated[name] = best_shift / fs

# ===================== 6. تثليث ثلاثي الأبعاد =====================
def trilateration_error(position, mic_positions, ref_distances):
    """
    دالة الخطأ للتثليث: تحسب الفرق بين المسافات المقدرة والفعلية
    """
    error = []
    for name, mic_pos in mic_positions.items():
        if name == 'Mic5':
            continue
        # المسافة المحسوبة من الموقع المقترح
        calc_dist = np.linalg.norm(position - mic_pos)
        # المسافة المقدرة من التأخير
        true_dist = ref_distances[name] + np.linalg.norm(position - mic_positions['Mic5'])
        error.append(calc_dist - true_dist)
    return np.array(error)

# حساب المسافات المرجعية من التأخيرات المقدرة
ref_distances = {}
for name in ['Mic2', 'Mic3', 'Mic4']:
    ref_distances[name] = delays_estimated[name] * c

# حل مشكلة التثليث باستخدام least_squares
initial_guess = np.array([0.25, 0.25, 0.15])
result = least_squares(
    lambda pos: trilateration_error(pos, mic_positions, ref_distances),
    initial_guess,
    method='lm'
)
estimated_position = result.x

# ===================== 7. محاكاة تأثير التكامل على ASR =====================
def simulate_asr_with_support(snr_db, support_enabled=True, reverberation=False):
    """
    محاكاة تأثير النظام المكاني على دقة التعرف على الكلام (ASR)
    - snr_db: نسبة الإشارة إلى الضوضاء (ديسيبل)
    - support_enabled: تفعيل/تعطيل دعم النظام المكاني
    - reverberation: تفعيل/تعطيل تأثير الصدى
    """
    # محاكاة الخطأ الأساسي (WER - Word Error Rate)
    base_error = np.random.normal(0.15, 0.05)  # 15% خطأ أساسي
    
    # تأثير الضوضاء على الخطأ
    noise_factor = max(0, (20 - snr_db) / 20)  # كلما انخفض SNR، زاد الخطأ
    error_without_support = min(0.8, base_error + noise_factor * 0.3)
    
    if support_enabled:
        # تحسن بفضل النظام المكاني (توجيه الحزمة وتنقية الإشارة)
        improvement = np.random.uniform(0.2, 0.4)  # تحسن 20-40%
        error_with_support = error_without_support * (1 - improvement)
    else:
        error_with_support = error_without_support
    
    # تأثير الصدى (يضيف خطأ إضافياً)
    if reverberation:
        error_with_support *= 1.2
        error_without_support *= 1.2
    
    return max(0, min(1, error_with_support)), max(0, min(1, error_without_support))

# اختبار سيناريوهات مختلفة
snr_values = [0, 5, 10, 15, 20, 25]  # ديسيبل
results = []

for snr in snr_values:
    err_with, err_without = simulate_asr_with_support(snr, support_enabled=True, reverberation=False)
    results.append((snr, err_with, err_without))

# ===================== 8. عرض النتائج =====================
print("========== نتائج المحاكاة (مع خوارزمية تنوير) ==========")
print(f"الموقع الحقيقي: {source_true}")
print(f"الموقع المقدر: {estimated_position}")
print(f"الخطأ المكاني (Euclidean): {np.linalg.norm(source_true - estimated_position):.4f} متر")

print("\n--- فروق التأخير المقدرة ---")
for name in ['Mic2', 'Mic3', 'Mic4']:
    true_delay = np.linalg.norm(source_true - mic_positions[name]) / c
    true_delay -= np.linalg.norm(source_true - mic_positions['Mic5']) / c
    print(f"{name}: حقيقي = {true_delay:.6f} ثانية, مقدر = {delays_estimated[name]:.6f} ثانية")

# ===================== 9. الرسوم البيانية =====================

# الرسم البياني 1: مقارنة الإشارات
plt.figure(figsize=(12, 4))
plt.plot(t[:1600], ref_signal[:1600], label='المرجع (Mic5)', alpha=0.8)
plt.plot(t[:1600], mic_signals['Mic2'][:1600], label='Mic2 (محيطي)', alpha=0.8)
plt.xlabel('الزمن (ثانية)')
plt.ylabel('السعة')
plt.title('مقارنة الإشارة المرجعية وإشارة ميكروفون محيطي (أول 100 مللي ثانية)')
plt.legend()
plt.grid(True)
plt.show()

# الرسم البياني 2: منحنى الخطأ مقابل التدوير (تنوير)
best_shift, min_err, shifts, errors = estimate_delay_tanweer(
    ref_signal, mic_signals['Mic2'], max_shift_samples
)
plt.figure(figsize=(10, 5))
plt.plot(shifts, errors)
plt.axvline(x=best_shift, color='r', linestyle='--', label=f'أفضل تدوير: {best_shift} عينة')
plt.xlabel('مقدار التدوير (عينات)')
plt.ylabel('مجموع الفروق المطلقة (بعد تطبيع L2)')
plt.title('منحنى الخطأ مقابل التدوير باستخدام خوارزمية تنوير (Mic2)')
plt.legend()
plt.grid(True)
plt.show()

# الرسم البياني 3: مخطط ثلاثي الأبعاد
fig = plt.figure(figsize=(10, 8))
ax = fig.add_subplot(111, projection='3d')

# رسم الميكروفونات
for name, pos in mic_positions.items():
    ax.scatter(*pos, s=100, label=f'{name}', color='blue')

# رسم المصدر الحقيقي والمقدر
ax.scatter(*source_true, s=200, color='green', label='المصدر الحقيقي')
ax.scatter(*estimated_position, s=200, color='red', label='المصدر المقدر (تنوير)')

ax.set_xlabel('X (متر)')
ax.set_ylabel('Y (متر)')
ax.set_zlabel('Z (متر)')
ax.set_title('مواقع الميكروفونات والمصدر (ثلاثي الأبعاد) مع خوارزمية تنوير')
ax.legend()
plt.show()

# الرسم البياني 4: تأثير التكامل على أداء ASR
plt.figure(figsize=(10, 6))
snr_vals = [r[0] for r in results]
err_with = [r[1] for r in results]
err_without = [r[2] for r in results]

plt.plot(snr_vals, err_with, 'o-', label='مع دعم النظام المكاني', color='green', linewidth=2)
plt.plot(snr_vals, err_without, 's-', label='بدون دعم النظام المكاني', color='red', linewidth=2)
plt.xlabel('نسبة الإشارة إلى الضوضاء (SNR) [ديسيبل]')
plt.ylabel('معدل خطأ الكلمات (WER)')
plt.title('تأثير النظام المكاني على دقة التعرف على الكلام (ASR)')
plt.legend()
plt.grid(True)
plt.ylim(0, 0.8)
plt.show()

# الرسم البياني 5: تحسن الأداء مع التكامل
improvements = [(err_without[i] - err_with[i]) / err_without[i] * 100 for i in range(len(snr_vals))]
plt.figure(figsize=(10, 6))
plt.bar([str(snr) for snr in snr_vals], improvements, color='blue', alpha=0.7)
plt.xlabel('نسبة الإشارة إلى الضوضاء (SNR) [ديسيبل]')
plt.ylabel('تحسن الدقة (%)')
plt.title('نسبة تحسن دقة التعرف على الكلام بفضل النظام المكاني')
plt.grid(True, axis='y')
plt.show()
