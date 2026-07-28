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

constexpr float kInitialZoomFactor = 0.30f; 
constexpr float kMinZoomLimit      = 0.03f; 
constexpr float kMaxZoomLimit      = 0.85f; 

std::atomic<bool> g_active{false};
std::atomic<bool> g_releasing{false};

std::atomic<float> g_targetFactor{kNeutralFactor};
float g_currentFactor = kNeutralFactor;

// Tracking waktu untuk animasi release
using Clock = std::chrono::steady_clock;
Clock::time_point g_releaseStartTime;
float g_releaseStartFactor = kNeutralFactor;
float g_releaseDurationMs = 150.0f;

float Clamp(float value) {
    return std::clamp(value, kMinZoomLimit, kMaxZoomLimit);
}

// Formula Cubic Ease-Out: Melambat secara mulus persis sebelum menyentuh FOV normal (1.0)
float EaseOutCubic(float t) {
    float f = 1.0f - t;
    return 1.0f - (f * f * f);
}

void PlaySpyglassSound(bool isStart) {
    if (!config::g_settings.enableSpyglassSound) return;
    if (isStart) {
        core::Log().info("ZoomController: Playing spyglass.use sound");
    } else {
        core::Log().info("ZoomController: Playing spyglass.stop_using sound");
    }
}

void UpdateHandVisibility(bool hide) {
    if (!config::g_settings.hideHandOnZoom) return;
    if (hide) {
        core::Log().info("ZoomController: Hiding player hand");
    } else {
        core::Log().info("ZoomController: Restoring player hand");
    }
}

} // namespace

void BeginZoom() {
    g_targetFactor.store(kInitialZoomFactor, std::memory_order_relaxed);
    g_releasing.store(false, std::memory_order_relaxed);
    g_active.store(true, std::memory_order_relaxed);

    PlaySpyglassSound(true);
    UpdateHandVisibility(true);
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
    
    // Kalkulasi durasi animasi berdasarkan setting kecepatan (Zoom Speed 1-10)
    float animSpeedSetting = static_cast<float>(config::g_settings.zoomAnimSpeed);
    g_releaseDurationMs = std::clamp(300.0f - (animSpeedSetting * 20.0f), 80.0f, 280.0f);

    g_releasing.store(true, std::memory_order_relaxed);
    PlaySpyglassSound(false);
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

        if (progress >= 1.0f) {
            // Animasi selesai: Tepat di 1.0f (100% FOV Normal Player), kembalikan kontrol ke game
            g_currentFactor = kNeutralFactor;
            camera_hook::SetOverride(kNeutralFactor);

            g_active.store(false, std::memory_order_relaxed);
            g_releasing.store(false, std::memory_order_relaxed);
            
            UpdateHandVisibility(false); 
            camera_hook::ClearOverride();
            return;
        }

        // Terapkan kurva Ease-Out
        float easedProgress = EaseOutCubic(progress);
        g_currentFactor = g_releaseStartFactor + (kNeutralFactor - g_releaseStartFactor) * easedProgress;
    } else {
        // Saat tombol ditahan/di-drag: transisi halus menuju target zoom
        float animSpeedSetting = static_cast<float>(config::g_settings.zoomAnimSpeed);
        float speedMultiplier = std::clamp(animSpeedSetting * 0.04f, 0.05f, 0.4f);
        float target = g_targetFactor.load(std::memory_order_relaxed);
        
        g_currentFactor += (target - g_currentFactor) * speedMultiplier;
    }

    camera_hook::SetOverride(g_currentFactor);
}

bool IsActive() {
    return g_active.load(std::memory_order_relaxed);
}

} // namespace zoom_controller
