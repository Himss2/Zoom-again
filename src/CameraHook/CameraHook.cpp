#include "CameraHook/CameraHook.hpp"

#include "Core/Config.hpp"
#include "Core/ModContext.hpp"
#include "ZoomController/ZoomController.hpp"

#include <pl/memory/Hook.hpp>
#include <pl/memory/Vtable.hpp>
#include <atomic>
#include <chrono>
#include <cstring>

namespace camera_hook {
namespace {

constexpr const char* kTypeInfoCameraAPI  = "9CameraAPI";
constexpr size_t      kTryGetFOVSlot      = 7;

constexpr const char* kTypeInfoOptions    = "7Options";
constexpr const char* kMinecraftModule   = "libminecraftpe.so";

using TryGetFOVFn   = uint64_t (*)(void*);
using GetHideHandFn = bool (*)(void*);

TryGetFOVFn   g_origTryGetFOV   = nullptr;
GetHideHandFn g_origGetHideHand = nullptr;

void* g_targetCamera   = nullptr;
void* g_targetHideHand = nullptr;

std::atomic<bool>  g_hasOverride{false};
std::atomic<float> g_overrideValue{1.0f};

std::function<void()> g_tickCallback;

uint64_t PackFov(bool hasValue, float value) {
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    uint64_t hi = hasValue ? 1u : 0u;
    return (hi << 32) | bits;
}

uint64_t DetourFOV(void* thisPtr) {
    // =========================================================================
    // OPTIMASI: FRAME GUARD (Mencegah eksekusi g_tickCallback berulang per frame)
    // =========================================================================
    static auto lastTickTime = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    
    // Batasi tick callback maksimal 1x setiap 8.000 mikrodetik (~125 FPS max rate)
    auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(now - lastTickTime).count();
    if (elapsedUs >= 8000) {
        lastTickTime = now;
        if (g_tickCallback) {
            g_tickCallback();
        }
    }

    if (!g_hasOverride.load(std::memory_order_relaxed)) {
        return g_origTryGetFOV(thisPtr);
    }

    return PackFov(true, g_overrideValue.load(std::memory_order_relaxed));
}

bool DetourHideHand(void* thisPtr) {
    if (zoom_controller::IsActive() && config::g_settings.hideHandOnZoom) {
        return true; 
    }
    
    if (g_origGetHideHand) {
        return g_origGetHideHand(thisPtr);
    }
    return false;
}

} // namespace

bool Install() {
    auto& log = core::Log();

    // 1. Hook CameraAPI::tryGetFOV
    g_targetCamera = reinterpret_cast<void*>(
        pl::memory::resolveVtableFunction(kTypeInfoCameraAPI, kTryGetFOVSlot, kMinecraftModule));

    if (!g_targetCamera) {
        log.error("CameraHook: failed to resolve CameraAPI::tryGetFOV");
        return false;
    }

    void* origCamOut = nullptr;
    int resCam = pl::memory::hook(
        g_targetCamera,
        reinterpret_cast<void*>(DetourFOV),
        &origCamOut,
        pl::memory::HookPriority::Normal);

    if (resCam != 0) {
        log.error("CameraHook: pl::memory::hook (FOV) failed, code={}", resCam);
        g_targetCamera = nullptr;
        return false;
    }
    g_origTryGetFOV = reinterpret_cast<TryGetFOVFn>(origCamOut);

    // 2. Hook Options::getHideHand
    constexpr size_t kHideHandSlots[] = {27, 28, 29, 26, 25, 30, 31, 32, 23, 22, 24};
    
    for (size_t slot : kHideHandSlots) {
        g_targetHideHand = reinterpret_cast<void*>(
            pl::memory::resolveVtableFunction(kTypeInfoOptions, slot, kMinecraftModule));

        if (g_targetHideHand) {
            void* origHideOut = nullptr;
            int resHide = pl::memory::hook(
                g_targetHideHand,
                reinterpret_cast<void*>(DetourHideHand),
                &origHideOut,
                pl::memory::HookPriority::Normal);

            if (resHide == 0) {
                g_origGetHideHand = reinterpret_cast<GetHideHandFn>(origHideOut);
                log.info("CameraHook: successfully installed Options::getHideHand hook at vtable slot {}", slot);
                break;
            }
        }
    }

    if (!g_origGetHideHand) {
        log.warn("CameraHook: could not resolve or hook Options::getHideHand on candidate slots");
    }

    return true;
}

void Uninstall() {
    if (g_targetCamera) {
        pl::memory::unhook(g_targetCamera, reinterpret_cast<void*>(DetourFOV));
        g_targetCamera = nullptr;
    }
    if (g_targetHideHand) {
        pl::memory::unhook(g_targetHideHand, reinterpret_cast<void*>(DetourHideHand));
        g_targetHideHand = nullptr;
    }
}

void SetOverride(float factor) {
    g_overrideValue.store(factor, std::memory_order_relaxed);
    g_hasOverride.store(true, std::memory_order_relaxed);
}

void ClearOverride() {
    g_hasOverride.store(false, std::memory_order_relaxed);
}

void SetFrameTickCallback(std::function<void()> callback) {
    g_tickCallback = std::move(callback);
}

} // namespace camera_hook
