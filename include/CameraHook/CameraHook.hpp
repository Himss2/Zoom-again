#pragma once
// CameraHook/CameraHook.hpp
//
// Owns the CameraAPI::tryGetFOV hook and nothing else - no touch, no
// drag, no animation curves. ZoomController is the only module allowed
// to call SetOverride/ClearOverride (see docs/architecture.md).
//
// Hook target and value encoding are CONFIRMED facts recovered via DWARF
// analysis of LeviLauncher's own built-in Zoom mod - see
// docs/research-notes.md for the derivation. Do not re-derive these:
//
//   resolveVtableFunction("9CameraAPI", 7, "libminecraftpe.so")
//     -> CameraAPI::tryGetFOV, hook signature: uint64_t(void* thisPtr)
//
// The uint64_t splits into two 32-bit halves: high = 1 if an override
// is active, low = the override value as a float bit-pattern, acting as
// a divisor on the base FOV (larger = zoomed in, smaller = zoomed out).

#include <functional>

namespace camera_hook {

// Resolves CameraAPI::tryGetFOV and installs the hook. Returns false
// (and logs the reason) on failure. Call once, from mod load().
bool Install();

// Reverts the hook, if installed. Call from mod unload().
void Uninstall();

// Overrides FOV with the given divisor (see file header for what the
// value means) until ClearOverride() is called.
void SetOverride(float factor);

// Hands control back to the game's own FOV logic.
void ClearOverride();

// Registers a callback invoked once per frame, right before the hook
// decides whether to override - a convenient per-frame "tick" source
// for whichever module needs one (ZoomController's release animation,
// ZoomButton's redraw), without CameraHook needing to know what those
// modules are. Wired up by Core, not called by CameraHook itself.
void SetFrameTickCallback(std::function<void()> callback);

} // namespace camera_hook
