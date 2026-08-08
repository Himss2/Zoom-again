#include "ZoomController/ZoomController.hpp"

#include "CameraHook/CameraHook.hpp"
#include "Core/Config.hpp"

#include <atomic>
#include <cmath>
#include <algorithm>

namespace zoom_controller {
namespace {

constexpr float kInitialZoomFactor = 0.30f; // Zoom awal saat tombol ditekan
constexpr float kMinZoomLimit      = 0.03f; // Zoom maksimal (teleskopik dekat)
constexpr float kMaxZoomLimit      = 0.85f; // Zoom minimal

// How close currentFactor must get to the target before we snap and
// clear the override. This used to be an absolute threshold (0.92f)
// which left an 8% gap to jump instantly - very noticeable at low
// release speeds since the decay curve is imperceptibly slow right
// before the snap. Using a small epsilon on the remaining distance
// instead means the final "jump" is always tiny (0.5% of the full
// range), regardless of releaseSpeed.
constexpr float kReleaseEpsilon = 0.005f;

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

    // Ambil setting kecepatan animasi dari Mod Menu (1 - 10, default = 5)
    float speedSetting = static_cast<float>(config::g_settings.zoomAnimSpeed);

    // Pada setting default (5):
    // - releaseSpeed = 5 * 0.08f = 0.40f
    // - zoomInSpeed  = 5 * 0.05f = 0.25f
    float releaseSpeed = std::clamp(speedSetting * 0.08f, 0.08f, 0.80f);
    float zoomInSpeed  = std::clamp(speedSetting * 0.05f, 0.05f, 0.50f);

    if (isReleasing) {
        // =====================================================================
        // TRANSISI KEMBALI KE FOV PLAYER TANPA PATAH
        // =====================================================================
        g_currentFactor += (target - g_currentFactor) * releaseSpeed;

        // Berhenti kalau jarak sisa ke target sudah sangat kecil - bukan
        // nilai absolut tetap (0.92f lama), supaya "lompatan" terakhir
        // selalu tipis (0.5% dari rentang) berapa pun releaseSpeed-nya,
        // alih-alih selalu 8% seperti sebelumnya.
        if (std::abs(target - g_currentFactor) <= kReleaseEpsilon) {
            g_currentFactor = kNeutralFactor;
            g_active.store(false, std::memory_order_relaxed);
            g_releasing.store(false, std::memory_order_relaxed);

            camera_hook::ClearOverride();
            return;
        }
    } else {
        // Logika saat Zoom In (ditekan / di-drag)
        float diff = target - g_currentFactor;
        g_currentFactor += diff * zoomInSpeed;
    }

    camera_hook::SetOverride(g_currentFactor);
}

bool IsActive() {
    return g_active.load(std::memory_order_relaxed);
}

} // namespace zoom_controller
