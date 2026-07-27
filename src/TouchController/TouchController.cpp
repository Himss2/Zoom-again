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

// --- TIMER LONG PRESS UNTUK REPOSITION MODE ---
// Tentukan berapa detik tombol harus ditahan sebelum bisa dipindahkan.
// Ubah angka ini (misal 3.0f atau 5.0f detik) sesuai kenyamanan kamu.
constexpr float kLongPressDurationSec = 3.0f; 

std::chrono::steady_clock::time_point g_touchStartTime;
bool g_isRepositionMode = false; // True jika sudah melewati durasi long press

int g_trackedPointerId = -1;
float g_startX = 0.0f;
float g_startY = 0.0f;
float g_lastY = 0.0f;

bool OnTouch(const pl::input::TouchEvent& ev) {
    switch (ev.action) {
        case kActionDown:
            if (g_trackedPointerId == -1 && zoom_button::Contains(ev.x, ev.y)) {
                g_trackedPointerId = ev.pointerId;
                g_startX = ev.x;
                g_startY = ev.y;
                g_lastY = ev.y;
                
                // Catat waktu awal sentuhan & reset mode pindah
                g_touchStartTime = std::chrono::steady_clock::now();
                g_isRepositionMode = false;
                
                // LANGSUNG MULAI ZOOM (Instan saat disentuh)
                zoom_controller::BeginZoom();
                core::Log().info("TouchController: hold start (pointer {})", ev.pointerId);
                return true;
            }
            return false;

        case kActionMove:
            if (ev.pointerId == g_trackedPointerId) {
                auto now = std::chrono::steady_clock::now();
                std::chrono::duration<float> elapsed = now - g_touchStartTime;

                // Cek apakah jari sudah menahan tombol melebihi batas waktu kLongPressDurationSec
                if (!g_isRepositionMode && elapsed.count() >= kLongPressDurationSec) {
                    g_isRepositionMode = true;
                    
                    // Batalkan zoom karena user berniat memindahkan posisi tombol
                    zoom_controller::EndZoom(); 
                    core::Log().info("TouchController: Long press ({:.1f}s) -> Switched to reposition mode", elapsed.count());
                }

                // -------------------------------------------------------------
                // 1. MODE PINDAH TOMBOL (Terpemicu HANYA jika sudah tahan lama)
                // -------------------------------------------------------------
                if (g_isRepositionMode) {
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
                // 2. MODE ZOOM NORMAL (Langsung drag instan saat baru disentuh)
                // -------------------------------------------------------------
                float deltaY = g_lastY - ev.y;
                g_lastY = ev.y;
                zoom_controller::UpdateDrag(deltaY * kDragSign * kDragSensitivity);
                core::Log().info("TouchController: deltaY={}", deltaY);
                return true;
            }
            return false;

        case kActionUp:
        case kActionCancel:
            if (ev.pointerId == g_trackedPointerId) {
                g_trackedPointerId = -1;
                
                if (!g_isRepositionMode) {
                    zoom_controller::EndZoom();
                    core::Log().info("TouchController: hold end, releasing zoom");
                } else {
                    core::Log().info("TouchController: finished repositioning button");
                }
                
                g_isRepositionMode = false;
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
