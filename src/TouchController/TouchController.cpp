#include "TouchController/TouchController.hpp"

#include "Core/ModContext.hpp"
#include "ZoomController/ZoomController.hpp"
#include "ZoomButton/ZoomButton.hpp"

#include <pl/Input.hpp>
#include <cmath>

namespace touch_controller {
namespace {

constexpr int kActionDown   = 0;
constexpr int kActionUp     = 1;
constexpr int kActionMove   = 2;
constexpr int kActionCancel = 3;

constexpr float kDragSensitivity = 0.005f;
constexpr float kDragSign = -1.0f;

// --- TAMBAHAN VARIABEL UNTUK EDIT/DRAG THRESHOLD ---
constexpr float kMoveThreshold = 10.0f; // Jarak minimal dalam piksel untuk mulai mendeteksi geseran posisi
float g_startX = 0.0f;
float g_startY = 0.0f;
bool g_isDraggingButton = false;      // True jika user sedang menggeser posisi tombol

int g_trackedPointerId = -1;
float g_lastY = 0.0f;

bool OnTouch(const pl::input::TouchEvent& ev) {
    switch (ev.action) {
        case kActionDown:
            if (g_trackedPointerId == -1 && zoom_button::Contains(ev.x, ev.y)) {
                g_trackedPointerId = ev.pointerId;
                g_startX = ev.x;
                g_startY = ev.y;
                g_lastY = ev.y;
                g_isDraggingButton = false; // Reset status geser
                
                // Mulai zoom (bisa dibatalkan nanti jika ternyata user berniat menggeser tombol)
                zoom_controller::BeginZoom();
                core::Log().info("TouchController: hold start (pointer {})", ev.pointerId);
                return true;
            }
            return false;

        case kActionMove:
            if (ev.pointerId == g_trackedPointerId) {
                // Jika sedang dalam proses drag tombol (edit posisi)
                if (g_isDraggingButton) {
                    float dx = ev.x - g_startX;
                    float dy = ev.y - g_startY;
                    g_startX = ev.x;
                    g_startY = ev.y;

                    // Update posisi tombol secara real-time mengikuti jari
                    float newX = zoom_button::GetX() + dx;
                    float newY = zoom_button::GetY() + dy;
                    zoom_button::SetPosition(newX, newY);
                    return true;
                }

                // Cek apakah pergerakan awal melewati threshold (artinya user ingin geser tombol, bukan zoom)
                float totalMoveX = std::abs(ev.x - g_startX);
                float totalMoveY = std::abs(ev.y - g_startY);
                if (totalMoveX > kMoveThreshold || totalMoveY > kMoveThreshold) {
                    g_isDraggingButton = true;
                    zoom_controller::EndZoom(); // Batalkan zoom seketika!
                    core::Log().info("TouchController: switched to button reposition mode");
                    return true;
                }

                // Logika zoom normal jika tidak bergeser jauh
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
                
                if (!g_isDraggingButton) {
                    zoom_controller::EndZoom();
                    core::Log().info("TouchController: hold end, releasing");
                } else {
                    core::Log().info("TouchController: finished repositioning button");
                }
                
                g_isDraggingButton = false;
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
