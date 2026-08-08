#pragma once
// CameraHook/CameraHook.hpp
//
// Owns the CameraAPI::tryGetFOV hook and nothing else - no touch, no
// drag, no animation curves. ZoomController is the only module allowed
// to call SetOverride/ClearOverride (see docs/architecture.md).
//
// Frame ticking is now handled separately by FrameHook (hooks
// eglSwapBuffers directly) - CameraHook no longer provides a tick
// source, since piggybacking on tryGetFOV's call frequency caused the
// choppy release animation.
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
//
// Options::getHideHand's vtable slot is NOT YET CONFIRMED. Install()
// currently installs a diagnostic passthrough hook on every candidate
// slot instead of guessing one - see the "HideHandDiag" block in the
// .cpp and docs/research-notes.md for how to read the results.

namespace camera_hook {

// Resolves CameraAPI::tryGetFOV and installs the hook. Also installs
// the (currently diagnostic-only) Options::getHideHand hooks. Returns
// false (and logs the reason) only if the FOV hook fails - the
// hideHand hooks are best-effort and don't fail Install() as a whole.
// Call once, from mod enable().
bool Install();

// Reverts all hooks installed by Install(). Call from mod disable()/unload().
void Uninstall();

// Overrides FOV with the given divisor (see file header for what the
// value means) until ClearOverride() is called.
void SetOverride(float factor);

// Hands control back to the game's own FOV logic.
void ClearOverride();

} // namespace camera_hook
