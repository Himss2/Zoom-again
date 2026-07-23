#include "Camera/FovOverride.hpp"

#include "Core/ModContext.hpp"
#include "UI/ZoomZone.hpp"
#include "Zoom/ZoomState.hpp"

#include <pl/memory/Hook.hpp>
#include <pl/memory/Vtable.hpp>
#include <cmath>
#include <cstring>

namespace camera {
namespace {

constexpr const char* kTypeInfoCameraAPI = "9CameraAPI";
constexpr size_t      kTryGetFOVSlot     = 7;
constexpr const char* kMinecraftModule   = "libminecraftpe.so";

constexpr float kReleaseLerpSpeed = 0.15f;
constexpr float kReleaseSnapEps   = 0.01f;

using TryGetFOVFn = uint64_t (*)(void*);
TryGetFOVFn g_origTryGetFOV = nullptr;
void* g_target = nullptr;

uint64_t PackFov(bool hasValue, float value) {
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    uint64_t hi = hasValue ? 1u : 0u;
    return (hi << 32) | bits;
}

uint64_t Detour(void* thisPtr) {
    ui::DrawZoomZone(); // piggyback per-frame UI redraw on this hook

    if (!zoom::IsActive()) {
        return g_origTryGetFOV(thisPtr);
    }

    float current = zoom::GetFactor();

    if (zoom::IsReleasing()) {
        current += (zoom::kNeutralZoom - current) * kReleaseLerpSpeed;
        if (std::fabs(current - zoom::kNeutralZoom) < kReleaseSnapEps) {
            zoom::SetActive(false);
            zoom::SetReleasing(false);
            zoom::SetFactor(zoom::kNeutralZoom);
            return g_origTryGetFOV(thisPtr);
        }
        zoom::SetFactor(current);
    }

    return PackFov(true, current);
}

} // namespace

bool InstallFovHook() {
    auto& log = core::Log();

    g_target = reinterpret_cast<void*>(
        pl::memory::resolveVtableFunction(kTypeInfoCameraAPI, kTryGetFOVSlot, kMinecraftModule));

    if (!g_target) {
        log.error("Camera: failed to resolve CameraAPI::tryGetFOV - hook not installed");
        return false;
    }

    void* originalOut = nullptr;
    int result = pl::memory::hook(
        g_target,
        reinterpret_cast<void*>(Detour),
        &originalOut,
        pl::memory::HookPriority::Normal);

    if (result != 0) {
        log.error("Camera: pl::memory::hook failed, code={}", result);
        g_target = nullptr;
        return false;
    }

    g_origTryGetFOV = reinterpret_cast<TryGetFOVFn>(originalOut);
    log.info("Camera: hook installed on CameraAPI::tryGetFOV at 0x{:x}",
             reinterpret_cast<uintptr_t>(g_target));
    return true;
}

void UninstallFovHook() {
    if (g_target) {
        pl::memory::unhook(g_target, reinterpret_cast<void*>(Detour));
        g_target = nullptr;
    }
}

} // namespace camera
