#include "ZoomController/ZoomController.hpp"

#include "CameraHook/CameraHook.hpp"

#include <atomic>
#include <cmath>

namespace zoom_controller {
namespace {

constexpr float kZoomLerpSpeed     = 0.25f;
constexpr float kSnapEps           = 0.0005f;

constexpr float kInitialZoomFactor = 0.30f; // Zoom awal saat ditekan (~70% zoom in)
constexpr float kMinZoomLimit      = 0.03f; // Batas maksimal zoom in (ultra teleskopik)
constexpr float kMaxZoomLimit      = 0.85f; // Batas minimal zoom in

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
    // delta < 0 (drag ke atas) akan memperkecil targetFactor -> Zoom IN semakin dekat
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
    float diff = target - g_currentFactor;

    g_currentFactor += diff * kZoomLerpSpeed;

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
