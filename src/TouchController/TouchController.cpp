#include "TouchController/TouchController.hpp"

#include "Core/ModContext.hpp"
#include "ZoomController/ZoomController.hpp"
#include "ZoomButton/ZoomButton.hpp"

#include <pl/Input.hpp>

namespace touch_controller {
namespace {

constexpr int kActionMask        = 0xFF;
constexpr int kActionDown        = 0;
constexpr int kActionUp          = 1;
constexpr int kActionMove        = 2;
constexpr int kActionCancel      = 3;
constexpr int kActionPointerDown = 5;
constexpr int kActionPointerUp   = 6;

int g_trackedPointerId = -1;
float g_lastY = 0.0f;

bool OnTouch(const pl::input::TouchEvent& ev) {
    int action = ev.action & kActionMask;

    switch (action) {
        case kActionDown:
        case kActionPointerDown:
            // Hanya tangkap jika sentuhan berada di dalam area tombol ZM
            if (g_trackedPointerId == -1 && zoom_button::Contains(ev.x, ev.y)) {
                g_trackedPointerId = ev.pointerId;
                g_lastY = ev.y;
                zoom_controller::BeginZoom();
                return true; // Konsumsi event DOWN agar tidak memukul/menghancurkan blok di game
            }
            return false; // Jari lain (gerakan kamera/jalan) diteruskan penuh ke Minecraft

        case kActionMove:
            if (g_trackedPointerId != -1 && ev.pointerId == g_trackedPointerId) {
                float deltaY = ev.y - g_lastY; 
                g_lastY = ev.y;
                
                // Usap ke atas (deltaY negatif) -> menambah zoom (+factor)
                // Usap ke bawah (deltaY positif) -> mengurangi zoom (-factor)
                // Pengali 0.008f disesuaikan dengan rentang skala baru [1.0f - 6.0f]
                zoom_controller::UpdateDrag(-deltaY * 0.008f);
            }
            // CRITICAL: Return false agar pergerakan kamera dari jari kedua tetap berjalan lancar
            return false;

        case kActionUp:
        case kActionPointerUp:
        case kActionCancel:
            if (ev.pointerId == g_trackedPointerId) {
                g_trackedPointerId = -1;
                zoom_controller::EndZoom();
                return true; // Konsumsi event UP dari tombol zoom
            }
            return false;

        default:
            return false;
    }
}

} // namespace

void Install() {
    pl::input::registerTouchCallback(OnTouch);
    core::Log().info("TouchController: Multi-touch passthrough & smooth drag ready");
}

} // namespace touch_controller
