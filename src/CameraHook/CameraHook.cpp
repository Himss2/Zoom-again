#include "CameraHook/CameraHook.hpp"

#include "Core/Config.hpp"
#include "Core/ModContext.hpp"
#include "ZoomController/ZoomController.hpp"

#include <pl/memory/Hook.hpp>
#include <pl/memory/Vtable.hpp>
#include <atomic>
#include <cstring>

namespace camera_hook {
namespace {

constexpr const char* kTypeInfoCameraAPI      = "9CameraAPI";
constexpr size_t      kTryGetFOVSlot          = 7;

// PERBAIKAN: "ItemInHandRenderer" terdiri dari 18 karakter (bukan 16)
constexpr const char* kTypeInfoItemRenderer   = "18ItemInHandRenderer";
constexpr const char* kMinecraftModule        = "libminecraftpe.so";

using TryGetFOVFn  = uint64_t (*)(void*);
using RenderHandFn = void (*)(void* thisPtr, void* arg1, void* arg2, void* arg3);

TryGetFOVFn    g_origTryGetFOV   = nullptr;
RenderHandFn   g_origRenderHand  = nullptr;

void* g_targetCamera     = nullptr;
void* g_targetRenderHand = nullptr;

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
    if (g_tickCallback) {
        g_tickCallback();
    }

    if (!g_hasOverride.load(std::memory_order_relaxed)) {
        return g_origTryGetFOV(thisPtr);
    }

    return PackFov(true, g_overrideValue.load(std::memory_order_relaxed));
}

void DetourRenderHand(void* thisPtr, void* arg1, void* arg2, void* arg3) {
    // Jika Zoom aktif dan toggle Hide Hand di Mod Menu dinyalakan, batalkan render tangan
    if (zoom_controller::IsActive() && config::g_settings.hideHandOnZoom) {
        return; 
    }
    
    if (g_origRenderHand) {
        g_origRenderHand(thisPtr, arg1, arg2, arg3);
    }
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

    // 2. Hook ItemInHandRenderer dengan pemindaian slot (1, 2, 3, 0, 4)
    constexpr size_t kRenderCandidateSlots[] = {1, 2, 3, 0, 4};
    for (size_t slot : kRenderCandidateSlots) {
        g_targetRenderHand = reinterpret_cast<void*>(
            pl::memory::resolveVtableFunction(kTypeInfoItemRenderer, slot, kMinecraftModule));

        if (g_targetRenderHand) {
            void* origRenderOut = nullptr;
            int resRender = pl::memory::hook(
                g_targetRenderHand,
                reinterpret_cast<void*>(DetourRenderHand),
                &origRenderOut,
                pl::memory::HookPriority::Normal);

            if (resRender == 0) {
                g_origRenderHand = reinterpret_cast<RenderHandFn>(origRenderOut);
                log.info("CameraHook: successfully hooked ItemInHandRenderer at vtable slot {}", slot);
                break;
            }
        }
    }

    if (!g_origRenderHand) {
        log.warn("CameraHook: could not resolve or hook ItemInHandRenderer");
    }

    return true;
}

void Uninstall() {
    if (g_targetCamera) {
        pl::memory::unhook(g_targetCamera, reinterpret_cast<void*>(DetourFOV));
        g_targetCamera = nullptr;
    }
    if (g_targetRenderHand) {
        pl::memory::unhook(g_targetRenderHand, reinterpret_cast<void*>(DetourRenderHand));
        g_targetRenderHand = nullptr;
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
