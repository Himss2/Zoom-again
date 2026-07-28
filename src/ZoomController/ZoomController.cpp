#include "ZoomController/ZoomController.hpp"
#include "CameraHook/CameraHook.hpp"
#include "Core/Config.hpp"
#include "Core/ModContext.hpp"

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
    
    // Catatan: Untuk memainkan suara native Minecraft secara langsung, 
    // dibutuhkan panggilan simbol SoundPlayer/Level::playSound dari game.
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
    g_targetFactor.store(kNeutralFactor, std::memory_order_relaxed);
    g_releasing.store(true, std::memory_order_relaxed);

    PlaySpyglassSound(false);
}

void Tick() {
    if (!g_active.load(std::memory_order_relaxed)) {
        return;
    }

    float speedMultiplier = static_cast<float>(config::g_settings.zoomAnimSpeed) * 0.045f;
    float target = g_targetFactor.load(std::memory_order_relaxed);
    bool isReleasing = g_releasing.load(std::memory_order_relaxed);

    if (isReleasing) {
        float diff = target - g_currentFactor;
        float step = diff * speedMultiplier;

        constexpr float kMinStep = 0.02f;
        if (step < kMinStep) {
            step = kMinStep;
        }

        g_currentFactor += step;

        if (g_currentFactor >= 0.98f) {
            g_currentFactor = kNeutralFactor;
            
            camera_hook::SetOverride(kNeutralFactor);

            g_active.store(false, std::memory_order_relaxed);
            g_releasing.store(false, std::memory_order_relaxed);
            
            UpdateHandVisibility(false); 
            camera_hook::ClearOverride();
            return;
        }
    } else {
        float diff = target - g_currentFactor;
        g_currentFactor += diff * speedMultiplier;
    }

    camera_hook::SetOverride(g_currentFactor);
}

bool IsActive() {
    return g_active.load(std::memory_order_relaxed);
}

} // namespace zoom_controller
