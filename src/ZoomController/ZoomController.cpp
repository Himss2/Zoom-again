#include "ZoomController/ZoomController.hpp"

#include "CameraHook/CameraHook.hpp"

#include <atomic>
#include <cmath>

namespace zoom_controller {
namespace {

// =============================================================================
// OPTIMIZED ANIMATION SETTINGS (ZOOM.CPP STYLE)
// =============================================================================
constexpr float kZoomLerpSpeed     = 0.22f; 
constexpr float kSnapEps           = 0.0005f;

constexpr float kInitialZoomFactor = 0.35f; 
constexpr float kMinZoomLimit      = 0.0001f; 
constexpr float kMaxZoomLimit      = 1.0f;   

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
    // Trik dari zoom.cpp: Jika tidak aktif, langsung RETURN! 0% Overhead CPU
    if (!g_active.load(std::memory_order_relaxed)) {
        return;
    }

    float target = g_targetFactor.load(std::memory_order_relaxed);
    float diff = target - g_currentFactor;

    // Transisi Interpolasi Mulus
    g_currentFactor += diff * kZoomLerpSpeed;

    // Jika animasi selesai dan kembali ke FOV Normal (1.0f)
    if (g_releasing.load(std::memory_order_relaxed)) {
        if (std::fabs(g_currentFactor - kNeutralFactor) < kSnapEps) {
            g_currentFactor = kNeutralFactor;
            g_active.store(false, std::memory_order_relaxed);
            g_releasing.store(false, std::memory_order_relaxed);
            
            // Bersihkan override kamera agar Minecraft berjalan 100% Native
            camera_hook::ClearOverride();
            return;
        }
    }

    camera_hook::SetOverride(g_currentFactor);
}

bool IsActive() {
    return g_active.load(std::memory_order_relaxed);
}

} // namespace zoom_controller
