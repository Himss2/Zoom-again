#include "TouchController/TouchController.hpp"

#include "Core/ModContext.hpp"
#include "ZoomButton/ZoomButton.hpp"
#include "ZoomController/ZoomController.hpp"

#include <android/input.h>
#include <atomic>

// =============================================================================
// FORWARD DECLARATION API INPUT PRELOADER
// (Menghindari fatal error file header tidak ditemukan)
// =============================================================================
namespace pl::legacy {
    using TouchListener = bool (*)(int action, int pointerId, float x, float y);
    bool registerTouchListener(TouchListener listener);
    void unregisterTouchListener(TouchListener listener);
}

namespace touch_controller {
namespace {

// ID jari yang sedang menekan tombol Zoom (-1 = tidak ada)
std::atomic<int32_t> g_zoomPointerId{-1};
float g_lastTouchY = 0.0f;

// =============================================================================
// CALLBACK TOUCH EVENT (DENGAN MULTI-TOUCH PASSTHROUGH)
// =============================================================================
bool OnTouch(int action, int pointerId, float x, float y) {
    int maskedAction = action & AMOTION_EVENT_ACTION_MASK;

    // 1. Jari Menekan Layar (DOWN / POINTER_DOWN)
    if (maskedAction == AMOTION_EVENT_ACTION_DOWN || maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN) {
        if (g_zoomPointerId.load() == -1 && zoom_button::Contains(x, y)) {
            g_zoomPointerId.store(pointerId);
            g_lastTouchY = y;
            
            zoom_controller::BeginZoom();
            return true; // Konsumsi event khusus untuk jari tombol zoom
        }
    }

    // 2. Jari Menggeser (MOVE)
    else if (maskedAction == AMOTION_EVENT_ACTION_MOVE) {
        int currentZoomPointer = g_zoomPointerId.load();
        if (currentZoomPointer != -1 && pointerId == currentZoomPointer) {
            float deltaY = (y - g_lastTouchY) * 0.0015f;
            zoom_controller::UpdateDrag(deltaY);
            g_lastTouchY = y;
            return true; // Konsumsi event khusus untuk jari tombol zoom
        }
    }

    // 3. Jari Diangkat (UP / POINTER_UP / CANCEL)
    else if (maskedAction == AMOTION_EVENT_ACTION_UP || 
             maskedAction == AMOTION_EVENT_ACTION_POINTER_UP || 
             maskedAction == AMOTION_EVENT_ACTION_CANCEL) {
        int currentZoomPointer = g_zoomPointerId.load();
        if (currentZoomPointer != -1 && pointerId == currentZoomPointer) {
            g_zoomPointerId.store(-1);
            zoom_controller::EndZoom();
            return true; // Konsumsi event khusus untuk jari tombol zoom
        }
    }

    // Return false agar jari lain (seperti menggeser kamera / rotasi pandangan)
    // DITERUSKAN 100% KE MINECRAFT!
    return false;
}

} // namespace

bool Install() {
    auto& log = core::Log();

    bool ok = pl::legacy::registerTouchListener(OnTouch);
    if (!ok) {
        log.error("TouchController: Gagal mendaftarkan Touch Listener");
        return false;
    }

    log.info("TouchController: Berhasil terpasang dengan Multi-Touch Passthrough");
    return true;
}

void Uninstall() {
    pl::legacy::unregisterTouchListener(OnTouch);
    g_zoomPointerId.store(-1);
}

} // namespace touch_controller
