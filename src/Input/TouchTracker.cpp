#include "Input/TouchTracker.hpp"

#include "Core/ModContext.hpp"
#include "UI/ZoomZone.hpp"
#include "Zoom/ZoomState.hpp"

#include <pl/Input.hpp>

namespace input {
namespace {

// Standard Android MotionEvent action constants (assumed to match
// TouchEvent.action).
constexpr int kActionDown   = 0;
constexpr int kActionUp     = 1;
constexpr int kActionMove   = 2;
constexpr int kActionCancel = 3;

constexpr float kDragSensitivity = 0.01f; // zoom-factor change per pixel of vertical drag
constexpr float kDragSign        = 1.0f;  // flip to -1.0f if direction is inverted on-device

float g_lastTouchY = 0.0f;

bool OnTouch(const pl::input::TouchEvent& ev) {
    int owner = zoom::GetOwnerPointerId();

    switch (ev.action) {
        case kActionDown: {
            if (owner == -1 && ui::PointInZoomZone(ev.x, ev.y)) {
                zoom::SetOwnerPointerId(ev.pointerId);
                zoom::SetFactor(zoom::kNeutralZoom);
                zoom::SetReleasing(false);
                zoom::SetActive(true);
                g_lastTouchY = ev.y;
                core::Log().info("Input: hold start (pointer {})", ev.pointerId);
                return true;
            }
            return false;
        }

        case kActionMove: {
            if (owner != -1 && ev.pointerId == owner) {
                float deltaY = g_lastTouchY - ev.y; // drag up -> y decreases -> positive
                g_lastTouchY = ev.y;
                zoom::AdjustFactor(deltaY * kDragSign * kDragSensitivity);
                core::Log().info("Input: deltaY={} -> factor={}", deltaY, zoom::GetFactor());
                return true;
            }
            return false;
        }

        case kActionUp:
        case kActionCancel: {
            if (owner != -1 && ev.pointerId == owner) {
                zoom::SetOwnerPointerId(-1);
                zoom::SetReleasing(true);
                core::Log().info("Input: hold end, releasing");
                return true;
            }
            return false;
        }

        default:
            return false;
    }
}

} // namespace

void InstallTouchTracking() {
    pl::input::registerTouchCallback(OnTouch);
    core::Log().info("Input: touch tracking installed");
}

} // namespace input
