#include "ZoomController/ZoomController.hpp"

#include "CameraHook/CameraHook.hpp"

#include <atomic>
#include <cmath>

namespace zoom_controller {
namespace {

constexpr float kNeutralFactor     = 1.0f;  
constexpr float kInitialZoomFactor = 0.30f; 
constexpr float kMinZoomLimit      = 0.03f; 
constexpr float kMaxZoomLimit      = 0.85f; 

std::atomic<bool> g_active{false};
std::atomic<bool> g_releasing{false};

std::atomic<float> g_targetFactor{kNeutralFactor};
float g_currentFactor = kNeutralFactor;

float Clamp(float value) {
    if (value < kMinZoomLimit) return kMinZoomLimit;
    if (value > kMaxZoomLimit) return kMaxZoomLimit;
    return value;
}

} // namespace

void BeginZoom() {
    g_targetFactor.store(kInitialZoomFactor, std::memory_order_relaxed);
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
    g_targetFactor.store(kNeutralFactor, std::memory_order_relaxed);
    g_releasing.store(true, std::memory_order_relaxed);
}

void Tick() {
    if (!g_active.load(std::memory_order_relaxed)) {
        return;
    }

    float target = g_targetFactor.load(std::memory_order_relaxed);
    bool isReleasing = g_releasing.load(std::memory_order_relaxed);

    if (isReleasing) {
        // =====================================================================
        // FIX: TRANSISI KEMBALI KE FOV PLAYER TANPA PATAH
        // =====================================================================
        // Kecepatan kembalinya camera FOV
        g_currentFactor += (target - g_currentFactor) * 0.40f;

        // Ambang batas (threshold) pelepasan hook diperbesar (0.92f)
        // Agar hook dilepas SAAT kamera sedang bergerak cepat menuju normal,
        // sehingga game engine Minecraft melanjutkan sisa pergerakan FOV-nya
        // sendiri tanpa ada jeda/tahanan kaku di 1.0f!
        if (g_currentFactor >= 0.92f) {
            g_currentFactor = kNeutralFactor;
            g_active.store(false, std::memory_order_relaxed);
            g_releasing.store(false, std::memory_order_relaxed);
            
            // Lepas override hook kamera secara mulus
            camera_hook::ClearOverride();
            return;
        }
    } else {
        // Logika saat Zoom In (ditekan / di-drag)
        float diff = target - g_currentFactor;
        g_currentFactor += diff * 0.25f;
    }

    camera_hook::SetOverride(g_currentFactor);
}

bool IsActive() {
    return g_active.load(std::memory_order_relaxed);
}

} // namespace zoom_controller
