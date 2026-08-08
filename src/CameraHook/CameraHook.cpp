#include "CameraHook/CameraHook.hpp"

#include "Core/Config.hpp"
#include "Core/ModContext.hpp"
#include "ZoomController/ZoomController.hpp"

#include <pl/memory/Hook.hpp>
#include <pl/memory/Vtable.hpp>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

namespace camera_hook {
namespace {

constexpr const char* kTypeInfoCameraAPI  = "9CameraAPI";
constexpr size_t      kTryGetFOVSlot      = 7;

// Previously guessed as "7Options" (class Options) - wrong target
// entirely. BedrockTools' zoom.cpp confirms the real function is
// BaseOptionRegistry::getHideItemInHand(), resolved by them via
// signature scanning (not vtable), but resolveVtableFunction should
// still work if this IS a virtual function - trying the correct class
// name now instead of the wrong one.
constexpr const char* kTypeInfoHideHand   = "19BaseOptionRegistry"; // "BaseOptionRegistry" = 19 chars
constexpr const char* kMinecraftModule    = "libminecraftpe.so";

using TryGetFOVFn   = uint64_t (*)(void*);
using GetHideHandFn = bool (*)(void*);

TryGetFOVFn   g_origTryGetFOV   = nullptr;

void* g_targetCamera   = nullptr;

std::atomic<bool>  g_hasOverride{false};
std::atomic<float> g_overrideValue{1.0f};

uint64_t PackFov(bool hasValue, float value) {
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    uint64_t hi = hasValue ? 1u : 0u;
    return (hi << 32) | bits;
}

uint64_t DetourFOV(void* thisPtr) {
    if (!g_hasOverride.load(std::memory_order_relaxed)) {
        return g_origTryGetFOV(thisPtr);
    }

    return PackFov(true, g_overrideValue.load(std::memory_order_relaxed));
}

// =============================================================================
// getHideHand DIAGNOSTIC BLOCK - TEMPORARY
//
// Target class updated to "BaseOptionRegistry" (was wrongly "Options"
// before) based on BedrockTools' zoom.cpp, which hooks
// BaseOptionRegistry::getHideItemInHand(). Range widened to slots 0-40
// since we don't know the right slot for this class yet - if
// resolveVtableFunction can't resolve ANY slot for this class, that's
// a signal the function may not be virtual (BedrockTools resolves it
// via signature scanning, not vtable), and this whole approach would
// need to switch to signature-based resolution instead.
//
// Each slot just logs how many times it's been called and forwards to
// the original implementation unchanged - never alters behavior.
//
// HOW TO USE: build with this in place, play normally, and specifically
// draw a bow and block with a shield a few times. Then check logcat:
//   adb logcat | grep HideHandDiag
// Whichever slot's count visibly jumps in sync with those actions is
// the real getHideItemInHand. Slots that resolve but never get called,
// or that fire constantly regardless of your actions, are not it.
//
// Once you have that slot number, tell me and I'll give you the final
// version: delete this whole diagnostic block and replace it with a
// single hardcoded hook on the confirmed slot, exactly like tryGetFOV
// above.
// =============================================================================

template <size_t Slot>
struct HideHandDiagnostic {
    static inline GetHideHandFn original = nullptr;
    static inline std::atomic<uint64_t> callCount{0};

    static bool Detour(void* thisPtr) {
        auto count = callCount.fetch_add(1, std::memory_order_relaxed) + 1;
        core::Log().info("HideHandDiag: slot {} called (count={})", Slot, count);
        return original ? original(thisPtr) : false;
    }
};

template <size_t Slot>
bool InstallHideHandDiagnostic() {
    void* target = reinterpret_cast<void*>(
        pl::memory::resolveVtableFunction(kTypeInfoHideHand, Slot, kMinecraftModule));
    if (!target) return false;

    void* origOut = nullptr;
    int res = pl::memory::hook(
        target,
        reinterpret_cast<void*>(&HideHandDiagnostic<Slot>::Detour),
        &origOut,
        pl::memory::HookPriority::Normal);
    if (res != 0) return false;

    HideHandDiagnostic<Slot>::original = reinterpret_cast<GetHideHandFn>(origOut);
    core::Log().info("HideHandDiag: hooked slot {}", Slot);
    return true;
}

// Expands to InstallHideHandDiagnostic<0>(), <1>(), ... <N-1>() at
// compile time, so we don't have to hand-write 41 call lines.
template <size_t... Slots>
void InstallHideHandDiagnosticSlots(std::index_sequence<Slots...>) {
    (InstallHideHandDiagnostic<Slots>(), ...);
}

} // namespace

bool Install() {
    auto& log = core::Log();

    // 1. Hook CameraAPI::tryGetFOV (confirmed slot - real feature, must succeed)
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

    // 2. getHideHand - diagnostic only for now, see block comment above.
    // Best-effort: failures here don't fail Install() as a whole, since
    // the core zoom feature (FOV) already succeeded above. Slots 0-40
    // of BaseOptionRegistry are probed; check logcat for "HideHandDiag"
    // lines to find the real one.
    log.info("HideHandDiag: probing BaseOptionRegistry slots 0-40...");
    InstallHideHandDiagnosticSlots(std::make_index_sequence<41>{});

    return true;
}

void Uninstall() {
    if (g_targetCamera) {
        pl::memory::unhook(g_targetCamera, reinterpret_cast<void*>(DetourFOV));
        g_targetCamera = nullptr;
    }
    g_origTryGetFOV = nullptr;

    // Diagnostic hideHand hooks are intentionally left installed/leaked
    // for now since this whole block is temporary and gets deleted once
    // the real slot is confirmed - not meant to survive to a release build.
}

void SetOverride(float factor) {
    g_overrideValue.store(factor, std::memory_order_relaxed);
    g_hasOverride.store(true, std::memory_order_relaxed);
}

void ClearOverride() {
    g_hasOverride.store(false, std::memory_order_relaxed);
}

} // namespace camera_hook
