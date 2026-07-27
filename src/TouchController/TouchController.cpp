#include "TouchController/TouchController.hpp"

#include "Core/ModContext.hpp"
#include "ZoomController/ZoomController.hpp"
#include "ZoomButton/ZoomButton.hpp"

#include <pl/Input.hpp>
#include <chrono>

namespace touch_controller {
namespace {

constexpr int kActionDown   = 0;
constexpr int kActionUp     = 1;
constexpr int kActionMove   = 2;
constexpr int kActionCancel = 3;

constexpr float kDragSensitivity = 0.005f;
constexpr float kDragSign = -1.0f;

// --- DOUBLE TAP & POSITION LOCK LOGIC ---
constexpr long long kDoubleTapMaxDelayMs = 300; // Jeda maksimal antar ketukan untuk dibaca sebagai Double Tap (300ms)

bool g_isPositionLocked = true; // Default TERKUNCI (Aman untuk gameplay)
std::chrono::steady_clock::time_point g_lastTapTime;

int g_trackedPointerId = -1;
float g_startX = 0.0f;
float g_startY = 0.0f;
float g_lastY = 0.0f;

bool OnTouch(const pl::input::TouchEvent& ev) {
    auto now = std::chrono::steady_clock::now();

    switch (ev.action) {
        case kActionDown:
            if (g_trackedPointerId == -1 && zoom_button::Contains(ev.x, ev.y)) {
                g_trackedPointerId = ev.pointerId;
                g_startX = ev.x;
                g_startY = ev.y;
                g_lastY = ev.y;

                // Hitung jeda dari ketukan sebelumnya
                auto tapDeltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_lastTapTime).count();

                // -------------------------------------------------------------
                // CEK DOUBLE TAP: Jika diketuk 2x cepat (< 300ms)
                // -------------------------------------------------------------
                if (tapDeltaMs <= kDoubleTapMaxDelayMs) {
                    g_isPositionLocked = !g_isPositionLocked; // Toggle Lock/Unlock

                    if (g_isPositionLocked) {
                        core::Log().info("TouchController: [LOCKED] Position locked for Zoom gameplay");
                    } else {
                        // Jika baru saja masuk Mode Reposition, matikan zoom
                        zoom_controller::EndZoom();
                        core::Log().info("TouchController: [UNLOCKED] Double-tap detected -> Reposition mode ACTIVE");
                    }
                }

                // Jika tombol TERKUNCI -> Jalankan ZOOM
                if (g_isPositionLocked) {
                    zoom_controller::BeginZoom();
                    core::Log().info("TouchController: Zoom started");
                }

                return true;
            }
            return false;

        case kActionMove:
            if (ev.pointerId == g_trackedPointerId) {
                // -------------------------------------------------------------
                // 1. MODE PINDAH TOMBOL (HANYA AKTIF JIKA DI-DOUBLE TAP / UNLOCKED)
                // -------------------------------------------------------------
                if (!g_isPositionLocked) {
                    float dx = ev.x - g_startX;
                    float dy = ev.y - g_startY;
                    g_startX = ev.x;
                    g_startY = ev.y;

                    float newX = zoom_button::GetX() + dx;
                    float newY = zoom_button::GetY() + dy;
                    zoom_button::SetPosition(newX, newY);
                    return true;
                }

                // -------------------------------------------------------------
                // 2. MODE ZOOM NORMAL (Saat TERKUNCI / LOCKED)
                // -------------------------------------------------------------
                float deltaY = g_lastY - ev.y;
                g_lastY = ev.y;
                zoom_controller::UpdateDrag(deltaY * kDragSign * kDragSensitivity);
                return true;
            }
            return false;

        case kActionUp:
        case kActionCancel:
            if (ev.pointerId == g_trackedPointerId) {
                g_trackedPointerId = -1;
                g_lastTapTime = now; // Simpan waktu pelepasan jari untuk deteksi double-tap berikutnya

                if (g_isPositionLocked) {
                    zoom_controller::EndZoom();
                }

                return true;
            }
            return false;

        default:
            return false;
    }
}

} // namespace

void Install() {
    pl::input::registerTouchCallback(OnTouch);
    core::Log().info("TouchController: installed");
}

} // namespace touch_controller
