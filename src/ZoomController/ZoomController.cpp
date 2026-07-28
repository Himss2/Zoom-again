#include "ZoomController/ZoomController.hpp"
#include "CameraHook/CameraHook.hpp"
#include "Core/Config.hpp"
#include "Core/ModContext.hpp"

#include <atomic>
#include <chrono>
#include <cmath>
#include <algorithm>

namespace zoom_controller {
namespace {

// Note: kNeutralFactor (1.0f) sudah didefinisikan di ZoomController.hpp
constexpr float kInitialZoomFactor = 2.5f;  // Zoom awal saat tombol ZM ditekan
constexpr float kMinZoomInFactor   = 1.0f;  // Batas Zoom Out (Pas di Normal FOV)
constexpr float kMaxZoomInFactor   = 6.0f;  // Batas Maksimum Zoom In (6x)

std::atomic<bool> g_active{false};
std::atomic<bool> g_releasing{false};

std::atomic<float> g_targetFactor{kNeutralFactor};
float g_currentFactor = kNeutralFactor;

using Clock = std::chrono::steady_clock;
Clock::time_point g_releaseStartTime;
float g_releaseStartFactor = kNeutralFactor;
float g_releaseDurationMs = 180.0f;

float Clamp(float value) {
    return std::clamp(value, kMinZoomInFactor, kMaxZoomInFactor);
}

// Kurva animasi Ease-Out Cubic untuk transisi mulus
float EaseOutCubic(float t) {
    float f = 1.0f - t;
    return 1.0f - (f * f * f);
}

} // namespace

void BeginZoom() {
    g_targetFactor.store(kInitialZoomFactor, std::memory_order_relaxed);
    g_currentFactor = kInitialZoomFactor; // Langsung set agar tidak ada lompatan awal dari 1.0
    g_releasing.store(false, std::memory_order_relaxed);
    g_active.store(true, std::memory_order_relaxed);
}

void UpdateDrag(float delta) {
    if (!g_active.load(std::memory_order_relaxed) || g_releasing.load(std::memory_order_relaxed)) {
        return;
    }

    float currentTarget = g_targetFactor.load(std::memory_order_relaxed);
    float newTarget = Clamp(currentTarget + delta);
    g_targetFactor.store(newTarget, std::memory_order_relaxed);
}

void EndZoom() {
    if (!g_active.load(std::memory_order_relaxed)) return;

    g_releaseStartFactor = g_currentFactor;
    g_releaseStartTime = Clock::now();
    
    // Kalkulasi durasi animasi rilis berdasarkan setting zoomAnimSpeed di Mod Menu (1-10)
    float animSpeedSetting = static_cast<float>(config::g_settings.zoomAnimSpeed);
    g_releaseDurationMs = std::clamp(320.0f - (animSpeedSetting * 22.0f), 100.0f, 300.0f);

    g_releasing.store(true, std::memory_order_relaxed);
}

void Tick() {
    if (!g_active.load(std::memory_order_relaxed)) {
        return;
    }

    bool isReleasing = g_releasing.load(std::memory_order_relaxed);

    if (isReleasing) {
        auto now = Clock::now();
        float elapsedMs = std::chrono::duration<float, std::milli>(now - g_releaseStartTime).count();
        float progress = elapsedMs / g_releaseDurationMs;

        // Jika animasi selesai, kembalikan kontrol penuh ke Minecraft
        if (progress >= 1.0f) {
            g_currentFactor = kNeutralFactor;
            camera_hook::SetOverride(kNeutralFactor);

            g_active.store(false, std::memory_order_relaxed);
            g_releasing.store(false, std::memory_order_relaxed);
            
            camera_hook::ClearOverride();
            return;
        }

        // Terapkan kurva Ease-Out dari nilai zoom terakhir menuju 1.0f
        float easedProgress = EaseOutCubic(progress);
        g_currentFactor = g_releaseStartFactor + (kNeutralFactor - g_releaseStartFactor) * easedProgress;
    } else {
        // Smoothing halus saat jari sedang mengusap/drag naik-turun
        float animSpeedSetting = static_cast<float>(config::g_settings.zoomAnimSpeed);
        float lerpSpeed = std::clamp(animSpeedSetting * 0.05f, 0.1f, 0.4f);
        float target = g_targetFactor.load(std::memory_order_relaxed);
        
        g_currentFactor += (target - g_currentFactor) * lerpSpeed;
    }

    camera_hook::SetOverride(g_currentFactor);
}

bool IsActive() {
    return g_active.load(std::memory_order_relaxed);
}

} // namespace zoom_controller
