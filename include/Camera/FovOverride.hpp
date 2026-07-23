#pragma once
// Camera/FovOverride.hpp
//
// Hooks CameraAPI::tryGetFOV to override the game's FOV while a zoom is
// active, and animates it back to neutral on release.
//
// Hook target confirmed via DWARF in LeviLauncher's own built-in Zoom
// mod (libinbuiltmods.so, ships unstripped) - the exact compiled
// call-site constants for its resolveVtableFunction() call were
// recovered from GNU_call_site_value entries, not guessed:
//
//   resolveVtableFunction("9CameraAPI", 7, "libminecraftpe.so")
//     -> CameraAPI::tryGetFOV, hook signature: uint64_t(void* thisPtr)
//
// Value encoding confirmed empirically on-device: the uint64_t splits
// into two 32-bit halves - high = 1 if an override is active, low = the
// override value as a float bit-pattern, acting as a divisor on base
// FOV (larger = zoomed in, smaller = zoomed out).

namespace camera {

// Resolves CameraAPI::tryGetFOV and installs the override hook. Returns
// false (and logs the reason) if resolution or hooking fails.
bool InstallFovHook();

// Reverts the hook, if installed.
void UninstallFovHook();

} // namespace camera
