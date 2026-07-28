#include "ZoomController/ZoomController.hpp"

#include "CameraHook/CameraHook.hpp"

#include <atomic>
#include <cmath>

namespace zoom_controller {
namespace {

constexpr float kInitialZoomFactor = 0.30f; // Zoom awal saat tombol ditekan
constexpr float kMinZoomLimit      = 0.03f; // Zoom maksimal (teleskopik dekat)
constexpr float kMaxZoomLimit      = 0.85f; // Zoom minimal

// =============================================================================
// KONFIGURASI PENGATURAN MOD (Bisa diubah nilainya atau dihubungkan ke UI)
// =============================================================================
int g_zoomAnimSpeed        = 5;    // Slider Rentang 1 (Lambat) - 10 (Cepat)
bool g_enableSpyglassSound = true; // Toggle Suara Spyglass
bool g_hideHandOnZoom      = true; // Toggle Sembunyikan Tangan saat Zoom

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
    if (!g_enableSpyglassSound) return;
    
    // Logika pemutaran suara spyglass (mengikuti API audio Preloader/Game)
    if (isStart) {
        // Play sound "item.spyglass.use"
    } else {
        // Play sound "item.spyglass.stop_using"
    }
}

void UpdateHandVisibility(bool hide) {
    if (!g_hideHandOnZoom) return;

    // Logika menyembunyikan tangan (first-person hand) saat zoom aktif
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
    // HITUNG KECEPATAN ANIMASI BERDASARKAN SLIDER (Rentang 1 - 10)
    // 1  -> 0.04f (Mulus & Lambat)
    // 5  -> 0.20f (Kecepatan Sedang/Ideal)
    // 10 -> 0.40f (Cepat/Instan)
    // =========================================================================
    float speedMultiplier = static_cast<float>(g_zoomAnimSpeed) * 0.04f;

    float target = g_targetFactor.load(std::memory_order_relaxed);
    bool isReleasing = g_releasing.load(std::memory_order_relaxed);

    if (isReleasing) {
        // Zoom out menggunakan kecepatan lerp dari Slider
        g_currentFactor += (target - g_currentFactor) * speedMultiplier;

        // Ambang batas 0.92f agar hook dilepas saat kamera masih punya momentum
        if (g_currentFactor >= 0.92f) {
            g_currentFactor = kNeutralFactor;
            g_active.store(false, std::memory_order_relaxed);
            g_releasing.store(false, std::memory_order_relaxed);
            
            UpdateHandVisibility(false); // Munculkan tangan kembali saat zoom selesai
            camera_hook::ClearOverride();
            return;
        }
    } else {
        // Zoom in menggunakan kecepatan lerp dari Slider
        float diff = target - g_currentFactor;
        g_currentFactor += diff * speedMultiplier;
    }

    camera_hook::SetOverride(g_currentFactor);
}

bool IsActive() {
    return g_active.load(std::memory_order_relaxed);
}

} // namespace zoom_controller
