#include "ZoomController/ZoomController.hpp"

#include "CameraHook/CameraHook.hpp"

#include <atomic>
#include <cmath>

namespace zoom_controller {
namespace {

constexpr float kReleaseLerpSpeed = 0.15f;
constexpr float kReleaseSnapEps   = 0.01f;

std::atomic<bool> g_active{false};
std::atomic<bool> g_releasing{false};
std::atomic<float> g_factor{kNeutralFactor};

float Clamp(float value) {
    if (value < kMinFactor) return kMinFactor;
    if (value > kMaxFactor) return kMaxFactor;
    return value;
}

} // namespace

void BeginZoom() {
    g_factor.store(kNeutralFactor, std::memory_order_relaxed);
    g_releasing.store(false, std::memory_order_relaxed);
    g_active.store(true, std::memory_order_relaxed);
    camera_hook::SetOverride(kNeutralFactor);
}

void UpdateDrag(float delta) {
    if (!g_active.load(std::memory_order_relaxed)) {
        return;
    }
    float f = Clamp(g_factor.load(std::memory_order_relaxed) + delta);
    g_factor.store(f, std::memory_order_relaxed);
    camera_hook::SetOverride(f);
}

void EndZoom() {
    g_releasing.store(true, std::memory_order_relaxed);
}

void Tick() {
    if (!g_active.load(std::memory_order_relaxed)) {
        return;
    }
    if (!g_releasing.load(std::memory_order_relaxed)) {
        return;
    }

    float current = g_factor.load(std::memory_order_relaxed);
    current += (kNeutralFactor - current) * kReleaseLerpSpeed;

    if (std::fabs(current - kNeutralFactor) < kReleaseSnapEps) {
        g_active.store(false, std::memory_order_relaxed);
        g_releasing.store(false, std::memory_order_relaxed);
        g_factor.store(kNeutralFactor, std::memory_order_relaxed);
        camera_hook::ClearOverride();
        return;
    }

    g_factor.store(current, std::memory_order_relaxed);
    camera_hook::SetOverride(current);
}

bool IsActive() {
    return g_active.load(std::memory_order_relaxed);
}

} // namespace zoom_controller
