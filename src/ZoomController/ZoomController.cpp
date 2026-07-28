#include "ZoomController/ZoomController.hpp"
#include "CameraHook/CameraHook.hpp"
#include "Core/Config.hpp" // Terhubung langsung ke Mod Menu Config

#include <atomic>
#include <cmath>

namespace zoom_controller {
namespace {

constexpr float kInitialZoomFactor = 0.30f; // Zoom awal saat tombol ditekan
constexpr float kMinZoomLimit      = 0.03f; // Zoom maksimal (terdekat)
constexpr float kMaxZoomLimit      = 0.85f; // Zoom minimal
constexpr float kNeutralFactor     = 1.0f;  // FOV Normal bawaan game

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
    // Membaca pengaturan dari Mod Menu
    if (!config::g_settings.enableSpyglassSound) return;
    
    if (isStart) {
        // Play sound "item.spyglass.use"
    } else {
        // Play sound "item.spyglass.stop_using"
    }
}

void UpdateHandVisibility(bool hide) {
    // Membaca pengaturan dari Mod Menu
    if (!config::g_settings.hideHandOnZoom) return;

    // Logika menyembunyikan tangan saat zoom aktif
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

    // =========================================================================
    // HITUNG KECEPATAN ANIMASI BERDASARKAN SLIDER MOD MENU (Rentang 1 - 10)
    // 1  -> 0.05f (Sangat Mulus & Halus)
    // 5  -> 0.20f (Ideal)
    // 10 -> 0.45f (Responsif & Cepat)
    // =========================================================================
    float speedMultiplier = static_cast<float>(config::g_settings.zoomAnimSpeed) * 0.045f;

    float target = g_targetFactor.load(std::memory_order_relaxed);
    bool isReleasing = g_releasing.load(std::memory_order_relaxed);

    if (isReleasing) {
        // Smooth Lerp kembali ke 1.0f
        g_currentFactor += (target - g_currentFactor) * speedMultiplier;

        // DIUBAH: Menggunakan toleransi 0.995f agar transisi zoom-out tuntas 100% mulus tanpa kaget/patah
        if (g_currentFactor >= 0.995f) {
            g_currentFactor = kNeutralFactor;
            g_active.store(false, std::memory_order_relaxed);
            g_releasing.store(false, std::memory_order_relaxed);
            
            UpdateHandVisibility(false); // Munculkan tangan kembali
            camera_hook::ClearOverride();
            return;
        }
    } else {
        // Smooth Lerp saat Zoom In
        float diff = target - g_currentFactor;
        g_currentFactor += diff * speedMultiplier;
    }

    camera_hook::SetOverride(g_currentFactor);
}

bool IsActive() {
    return g_active.load(std::memory_order_relaxed);
}

} // namespace zoom_controller
