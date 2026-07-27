#include "ZoomController/ZoomController.hpp"

#include "CameraHook/CameraHook.hpp"

#include <atomic>
#include <cmath>

namespace zoom_controller {
namespace {

// =============================================================================
// ANIMATION SETTINGS
// =============================================================================
// kZoomLerpSpeed: Kecepatan animasi zoom (0.1f - 0.3f).
// - Semakin kecil (misal 0.10f) = Animasi makin lembut / cinematis.
// - Semakin besar (misal 0.30f) = Animasi makin responsif / cepat.
constexpr float kZoomLerpSpeed = 0.18f; 
constexpr float kSnapEps       = 0.0005f;

constexpr float kMinZoomLimit  = 0.0001f; 
constexpr float kMaxZoomLimit  = 1.0f;   

std::atomic<bool> g_active{false};
std::atomic<bool> g_releasing{false};

// Target FOV (posisi tujuan) vs Current FOV (posisi animasi kamera saat ini)
std::atomic<float> g_targetFactor{kNeutralFactor};
float g_currentFactor = kNeutralFactor;

float Clamp(float value) {
    if (value < kMinZoomLimit) return kMinZoomLimit;
    if (value > kMaxZoomLimit) return kMaxZoomLimit;
    return value;
}

} // namespace

void BeginZoom() {
    g_targetFactor.store(kNeutralFactor, std::memory_order_relaxed);
    g_releasing.store(false, std::memory_order_relaxed);
    g_active.store(true, std::memory_order_relaxed);
}

void UpdateDrag(float delta) {
    if (!g_active.load(std::memory_order_relaxed)) {
        return;
    }

    // Geser nilai target FOV, animasi akan menyusul di fungsi Tick()
    float newTarget = Clamp(g_targetFactor.load(std::memory_order_relaxed) + delta);
    g_targetFactor.store(newTarget, std::memory_order_relaxed);
}

void EndZoom() {
    // Kembalikan target FOV ke normal (1.0f) untuk animasi mereda
    g_targetFactor.store(kNeutralFactor, std::memory_order_relaxed);
    g_releasing.store(true, std::memory_order_relaxed);
}

void Tick() {
    if (!g_active.load(std::memory_order_relaxed)) {
        return;
    }

    float target = g_targetFactor.load(std::memory_order_relaxed);

    // -------------------------------------------------------------------------
    // SMOOTH LERP ANIMATION FRAME-BY-FRAME
    // -------------------------------------------------------------------------
    g_currentFactor += (target - g_currentFactor) * kZoomLerpSpeed;

    // Jika sedang dilepas (releasing) dan kamera sudah hampir kembali ke FOV normal
    if (g_releasing.load(std::memory_order_relaxed)) {
        if (std::fabs(g_currentFactor - kNeutralFactor) < kSnapEps) {
            g_currentFactor = kNeutralFactor;
            g_active.store(false, std::memory_order_relaxed);
            g_releasing.store(false, std::memory_order_relaxed);
            camera_hook::ClearOverride();
            return;
        }
    }

    // Terapkan FOV hasil animasi ke kamera
    camera_hook::SetOverride(g_currentFactor);
}

bool IsActive() {
    return g_active.load(std::memory_order_relaxed);
}

} // namespace zoom_controller
