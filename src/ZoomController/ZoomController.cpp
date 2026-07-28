#include "ZoomController/ZoomController.hpp"

#include "CameraHook/CameraHook.hpp"

#include <atomic>
#include <cmath>

namespace zoom_controller {
namespace {

constexpr float kInitialZoomFactor = 0.30f; // Zoom awal saat tombol ditekan
constexpr float kMinZoomLimit      = 0.03f; // Zoom maksimal (teleskopik dekat)
constexpr float kMaxZoomLimit      = 0.85f; // Zoom minimal

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
        // TRANSISI KEMBALI KE FOV PLAYER TANPA PATAH
        // =====================================================================
        g_currentFactor += (target - g_currentFactor) * 0.40f;

        // Ambang batas 0.92f agar hook dilepas saat kamera masih punya momentum
        if (g_currentFactor >= 0.92f) {
            g_currentFactor = kNeutralFactor;
            g_active.store(false, std::memory_order_relaxed);
            g_releasing.store(false, std::memory_order_relaxed);
            
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
