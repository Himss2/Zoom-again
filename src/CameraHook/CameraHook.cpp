#include "CameraHook/CameraHook.hpp"

#include "Core/ModContext.hpp"

#include <pl/memory/Hook.hpp>
#include <pl/memory/Vtable.hpp>
#include <atomic>
#include <cstring>

namespace camera_hook {
namespace {

constexpr const char* kTypeInfoCameraAPI = "9CameraAPI";
constexpr size_t      kTryGetFOVSlot     = 7;
constexpr const char* kMinecraftModule   = "libminecraftpe.so";

using TryGetFOVFn = uint64_t (*)(void*);
TryGetFOVFn g_origTryGetFOV = nullptr;
void* g_target = nullptr;

std::atomic<bool> g_hasOverride{false};
std::atomic<float> g_overrideValue{1.0f};

std::function<void()> g_tickCallback;

uint64_t PackFov(bool hasValue, float value) {
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    uint64_t hi = hasValue ? 1u : 0u;
    return (hi << 32) | bits;
}

uint64_t Detour(void* thisPtr) {
    if (g_tickCallback) {
        g_tickCallback();
    }

    if (!g_hasOverride.load(std::memory_order_relaxed)) {
        return g_origTryGetFOV(thisPtr);
    }

    return PackFov(true, g_overrideValue.load(std::memory_order_relaxed));
}

} // namespace

bool Install() {
    auto& log = core::Log();

    g_target = reinterpret_cast<void*>(
        pl::memory::resolveVtableFunction(kTypeInfoCameraAPI, kTryGetFOVSlot, kMinecraftModule));

    if (!g_target) {
        log.error("CameraHook: failed to resolve CameraAPI::tryGetFOV - hook not installed");
        return false;
    }

    void* originalOut = nullptr;
    int result = pl::memory::hook(
        g_target,
        reinterpret_cast<void*>(Detour),
        &originalOut,
        pl::memory::HookPriority::Normal);

    if (result != 0) {
        log.error("CameraHook: pl::memory::hook failed, code={}", result);
        g_target = nullptr;
        return false;
    }

    g_origTryGetFOV = reinterpret_cast<TryGetFOVFn>(originalOut);
    log.info("CameraHook: installed on CameraAPI::tryGetFOV at 0x{:x}",
             reinterpret_cast<uintptr_t>(g_target));
    return true;
}

void Uninstall() {
    if (g_target) {
        pl::memory::unhook(g_target, reinterpret_cast<void*>(Detour));
        g_target = nullptr;
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
