#include "ZoomController/ZoomController.hpp"

#include "CameraHook/CameraHook.hpp"

#include <atomic>
#include <cmath>

namespace zoom_controller {
namespace {

// =============================================================================
// ANIMATION SETTINGS
// =============================================================================
constexpr float kZoomLerpSpeed     = 0.18f; // Kecepatan transisi animasi
constexpr float kSnapEps           = 0.0005f;

constexpr float kNeutralFactor     = 1.0f;  // FOV Normal Game
constexpr float kInitialZoomFactor = 0.35f; // FOV Awal saat tombol BARU DITEKAN (Langsung Animasi Smooth)
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
    // Saat ditekan, target langsung diset ke kInitialZoomFactor.
    // g_currentFactor (yang masih 1.0f) akan otomatis ter-animasi mulus di Tick()
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
    // Saat dilepas, kembalikan target ke 1.0f agar ter-animasi balik secara smooth
    g_targetFactor.store(kNeutralFactor, std::memory_order_relaxed);
    g_releasing.store(true, std::memory_order_relaxed);
}

void Tick() {
    if (!g_active.load(std::memory_order_relaxed)) {
        return;
    }

    float target = g_targetFactor.load(std::memory_order_relaxed);

    // -------------------------------------------------------------------------
    // SMOOTH LERP INTERPOLATION (Setiap Frame)
    // -------------------------------------------------------------------------
    g_currentFactor += (target - g_currentFactor) * kZoomLerpSpeed;

    if (g_releasing.load(std::memory_order_relaxed)) {
        if (std::fabs(g_currentFactor - kNeutralFactor) < kSnapEps) {
            g_currentFactor = kNeutralFactor;
            g_active.store(false, std::memory_order_relaxed);
            g_releasing.store(false, std::memory_order_relaxed);
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
