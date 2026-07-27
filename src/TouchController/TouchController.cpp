#include "TouchController/TouchController.hpp"

#include "Core/ModContext.hpp"
#include "ZoomController/ZoomController.hpp"
#include "ZoomButton/ZoomButton.hpp"

#include <pl/Input.hpp>

namespace touch_controller {
namespace {

// Kode Action Event Android (termasuk Multi-Touch Pointer)
constexpr int kActionMask        = 0xFF;
constexpr int kActionDown        = 0;
constexpr int kActionUp          = 1;
constexpr int kActionMove        = 2;
constexpr int kActionCancel      = 3;
constexpr int kActionPointerDown = 5;
constexpr int kActionPointerUp   = 6;

constexpr float kDragSensitivity = 0.005f;
constexpr float kDragSign        = -1.0f;

int g_trackedPointerId = -1;
float g_lastY = 0.0f;

bool OnTouch(const pl::input::TouchEvent& ev) {
    // Isolasi jenis action menggunakan Bitmask Android Multi-touch
    int action = ev.action & kActionMask;

    switch (action) {
        case kActionDown:
        case kActionPointerDown:
            // Hanya kunci jika belum ada jari zoom yang terdeteksi
            if (g_trackedPointerId == -1 && zoom_button::Contains(ev.x, ev.y)) {
                g_trackedPointerId = ev.pointerId;
                g_lastY = ev.y;
                zoom_controller::BeginZoom();
                return true; // Konsumsi khusus jari tombol zoom
            }
            return false; // Jari lain diteruskan 100% ke Minecraft!

        case kActionMove:
            if (ev.pointerId == g_trackedPointerId) {
                float deltaY = g_lastY - ev.y;
                g_lastY = ev.y;
                zoom_controller::UpdateDrag(deltaY * kDragSign * kDragSensitivity);
                return true; // Konsumsi khusus drag zoom
            }
            return false; // Jari penggerak kamera diteruskan 100% ke Minecraft!

        case kActionUp:
        case kActionPointerUp:
        case kActionCancel:
            if (ev.pointerId == g_trackedPointerId) {
                g_trackedPointerId = -1;
                zoom_controller::EndZoom();
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
    core::Log().info("TouchController: installed with multi-touch support");
}

} // namespace touch_controller
