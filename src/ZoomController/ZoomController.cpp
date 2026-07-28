#include "ZoomController/ZoomController.hpp"
#include "CameraHook/CameraHook.hpp"
#include "Core/Config.hpp"

#include <pl/Runtime.hpp> // API untuk Sound & Hide Hand JNI / MC Call
#include <atomic>
#include <cmath>

namespace zoom_controller {
namespace {

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

void PlaySpyglassSound(bool isStart) {
    if (!config::g_settings.enableSpyglassSound) return;
    
    if (isStart) {
        pl::runtime::playLocalSound("item.spyglass.use", 1.0f, 1.0f);
    } else {
        pl::runtime::playLocalSound("item.spyglass.stop_using", 1.0f, 1.0f);
    }
}

void UpdateHandVisibility(bool hide) {
    if (!config::g_settings.hideHandOnZoom) return;
    
    // Toggle render first person arm / hand
    pl::runtime::setFirstPersonHandVisible(!hide);
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
    g_targetFactor.store(kNeutralFactor, std::memory_order_relaxed);
    g_releasing.store(true, std::memory_order_relaxed);

    PlaySpyglassSound(false);
}

void Tick() {
    if (!g_active.load(std::memory_order_relaxed)) {
        return;
    }

    // Hitung multiplier kecepatan berdasarkan Slider (1 = 0.05f [sangat mulus/lambat], 10 = 0.50f [sangat cepat])
    float speedMultiplier = static_cast<float>(config::g_settings.zoomAnimSpeed) * 0.05f;

    float target = g_targetFactor.load(std::memory_order_relaxed);
    bool isReleasing = g_releasing.load(std::memory_order_relaxed);

    if (isReleasing) {
        // Kecepatan zoom out dinamis mengikuti Slider
        g_currentFactor += (target - g_currentFactor) * speedMultiplier;

        if (g_currentFactor >= 0.92f) {
            g_currentFactor = kNeutralFactor;
            g_active.store(false, std::memory_order_relaxed);
            g_releasing.store(false, std::memory_order_relaxed);
            
            UpdateHandVisibility(false); // Munculkan tangan kembali
            camera_hook::ClearOverride();
            return;
        }
    } else {
        // Kecepatan zoom in dinamis mengikuti Slider
        float diff = target - g_currentFactor;
        g_currentFactor += diff * (speedMultiplier * 0.65f);
    }

    camera_hook::SetOverride(g_currentFactor);
}

bool IsActive() {
    return g_active.load(std::memory_order_relaxed);
}

} // namespace zoom_controller
