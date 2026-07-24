#include "TouchController/TouchController.hpp"

#include "Core/ModContext.hpp"
#include "ZoomController/ZoomController.hpp"

#include <pl/Input.hpp>

namespace touch_controller {
namespace {

// Standard Android MotionEvent action constants (assumed to match
// TouchEvent.action - confirmed working this way in the previous
// project).
constexpr int kActionDown   = 0;
constexpr int kActionUp     = 1;
constexpr int kActionMove   = 2;
constexpr int kActionCancel = 3;

constexpr float kDragSensitivity = 0.01f;

// Corrected sign: Step 1 confirmed on-device that a SMALLER
// ZoomController factor means zoomed IN (the value is a multiplier on
// base FOV, not a divisor as first assumed). Dragging up produces a
// positive deltaY (y decreases), and dragging up should zoom IN, so
// increasing deltaY must DECREASE the factor - hence a negative sign.
constexpr float kDragSign = -1.0f;

int g_trackedPointerId = -1;
float g_lastY = 0.0f;

bool OnTouch(const pl::input::TouchEvent& ev) {
    switch (ev.action) {
        case kActionDown:
            if (g_trackedPointerId == -1) {
                // STEP 2 SHORTCUT - see header comment: no zone to
                // hit-test against yet, so any first touch starts a hold.
                g_trackedPointerId = ev.pointerId;
                g_lastY = ev.y;
                zoom_controller::BeginZoom();
                core::Log().info("TouchController: hold start (pointer {})", ev.pointerId);
            }
            return false;

        case kActionMove:
            if (ev.pointerId == g_trackedPointerId) {
                float deltaY = g_lastY - ev.y; // positive when dragging up
                g_lastY = ev.y;
                zoom_controller::UpdateDrag(deltaY * kDragSign * kDragSensitivity);
                core::Log().info("TouchController: deltaY={}", deltaY);
            }
            return false;

        case kActionUp:
        case kActionCancel:
            if (ev.pointerId == g_trackedPointerId) {
                g_trackedPointerId = -1;
                zoom_controller::EndZoom();
                core::Log().info("TouchController: hold end, releasing");
            }
            return false;

        default:
            return false;
    }
}

} // namespace

void Install() {
    pl::input::registerTouchCallback(OnTouch);
    core::Log().info("TouchController: installed (Step 2 - triggers on any touch, no zone yet)");
}

} // namespace touch_controller
