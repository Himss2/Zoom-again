#include "TouchController/TouchController.hpp"

#include "Core/ModContext.hpp"
#include "ZoomController/ZoomController.hpp"
#include "ZoomButton/ZoomButton.hpp"

#include <pl/Input.hpp>

namespace touch_controller {
namespace {

constexpr int kActionDown   = 0;
constexpr int kActionUp     = 1;
constexpr int kActionMove   = 2;
constexpr int kActionCancel = 3;

constexpr float kDragSensitivity = 0.01f;
constexpr float kDragSign = -1.0f;

int g_trackedPointerId = -1;
float g_lastY = 0.0f;

bool OnTouch(const pl::input::TouchEvent& ev) {
    switch (ev.action) {
        case kActionDown:
            if (g_trackedPointerId == -1 && zoom_button::Contains(ev.x, ev.y)) {
                g_trackedPointerId = ev.pointerId;
                g_lastY = ev.y;
                zoom_controller::BeginZoom();
                core::Log().info("TouchController: hold start (pointer {})", ev.pointerId);
                return true;
            }
            return false;

        case kActionMove:
            if (ev.pointerId == g_trackedPointerId) {
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
                zoom_controller::EndZoom();
                core::Log().info("TouchController: hold end, releasing");
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
    core::Log().info("TouchController: installed (Step 3 - gated behind ZoomButton::Contains)");
}

} // namespace touch_controller
