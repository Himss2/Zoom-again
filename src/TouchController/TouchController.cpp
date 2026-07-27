#include "TouchController/TouchController.hpp"
#include "ZoomButton/ZoomButton.hpp"
#include "ZoomController/ZoomController.hpp"

#include <android/input.h>
#include <atomic>

namespace touch_controller {
namespace {

// Menyimpan ID jari spesifik yang sedang menekan tombol Zoom (-1 = tidak ada)
std::atomic<int32_t> g_zoomPointerId{-1};
float g_lastTouchY = 0.0f;

} // namespace

// =============================================================================
// LOGIKA INPUT TOUCH MULTI-TOUCH PASSTHROUGH
// =============================================================================
bool HandleTouchEvent(int action, int pointerId, float x, float y, bool (*origTouchFunc)(int, int, float, float)) {
    int maskedAction = action & AMOTION_EVENT_ACTION_MASK;

    // -------------------------------------------------------------------------
    // 1. JARI MENYENTUH LAYAR (ACTION_DOWN / ACTION_POINTER_DOWN)
    // -------------------------------------------------------------------------
    if (maskedAction == AMOTION_EVENT_ACTION_DOWN || maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN) {
        // Cek apakah jari ini menyentuh area tombol "ZM"
        if (g_zoomPointerId.load() == -1 && zoom_button::Contains(x, y)) {
            g_zoomPointerId.store(pointerId);
            g_lastTouchY = y;
            
            // Aktifkan Fitur Zoom
            zoom_controller::BeginZoom();
            
            // KONSUMSI EVENT: Jangan teruskannya ke Minecraft agar karakter tidak memukul/menghancurkan blok
            return true; 
        }
    }

    // -------------------------------------------------------------------------
    // 2. JARI BERGERAK / DI-DRAG (ACTION_MOVE)
    // -------------------------------------------------------------------------
    else if (maskedAction == AMOTION_EVENT_ACTION_MOVE) {
        int currentZoomPointer = g_zoomPointerId.load();
        
        // Jika jari yang bergerak ini ADALAH jari yang menekan tombol Zoom:
        if (currentZoomPointer != -1 && pointerId == currentZoomPointer) {
            float deltaY = (y - g_lastTouchY) * 0.0015f; // Sensitivitas drag zoom
            zoom_controller::UpdateDrag(deltaY);
            g_lastTouchY = y;
            
            // Konsumsi input drag zoom ini
            return true;
        }
    }

    // -------------------------------------------------------------------------
    // 3. JARI DIANGKAT (ACTION_UP / ACTION_POINTER_UP / ACTION_CANCEL)
    // -------------------------------------------------------------------------
    else if (maskedAction == AMOTION_EVENT_ACTION_UP || 
             maskedAction == AMOTION_EVENT_ACTION_POINTER_UP || 
             maskedAction == AMOTION_EVENT_ACTION_CANCEL) {
        
        int currentZoomPointer = g_zoomPointerId.load();
        
        // Jika jari yang diangkat adalah jari tombol Zoom:
        if (currentZoomPointer != -1 && pointerId == currentZoomPointer) {
            g_zoomPointerId.store(-1);
            
            // Matikan Zoom
            zoom_controller::EndZoom();
            
            return true;
        }
    }

    // -------------------------------------------------------------------------
    // CRITICAL FIX: JARI KEDUA (MENGARAHKAN KAMERA) DITERUSKAN KE MINECRAFT!
    // -------------------------------------------------------------------------
    // Jika touch event ini BUKAN berasal dari jari tombol zoom, 
    // langsung berikan ke fungsi sentuh asli Minecraft (origTouchFunc).
    if (origTouchFunc) {
        return origTouchFunc(action, pointerId, x, y);
    }

    return false;
}

} // namespace touch_controller
