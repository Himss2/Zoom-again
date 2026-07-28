# Changelog

## Unreleased

- UI: replaced the pixel-notch button look with a rounded-rectangle
  style (separate outline + fill layers, corner smoothing via stacked
  horizontal strips) - closer to third-party mod UI conventions
  (background/outline/text opacity as independent parameters).
- Investigated `libBedrockTools-9.so` for UI/style reference: confirmed
  it's also built on the `pl::` SDK (`pl::modmenu::submitDrawCommands`
  symbol present); no distinct `Circle`/`RoundedRect` draw-command type
  found, suggesting rect composition is the standard approach for
  rounded shapes on this SDK.
- Root-caused the previously-blocking SIGSEGV in
  `pl::modmenu::registerModule`: fixed by installing `CameraHook`
  (and registering the ModMenu module) from `enable()` instead of
  `load()`, so it doesn't race the launcher's Inbuilt Zoom Mod
  `nativeInit()`. Previous "thread-pool race between mods" theory
  superseded.
- Added full config: zoom animation speed, hide-hand-on-zoom toggle,
  button opacity, position (X/Y), and scale - all persisted and
  editable via Mod Menu.
- **Known bug:** `hideHandOnZoom` toggle has no visible effect.
  `Options::getHideHand` vtable slot is being guessed from a candidate
  list rather than confirmed (unlike `CameraAPI::tryGetFOV`, which was
  DWARF-verified) - likely hooking the wrong function. Needs proper
  verification before this feature can be considered working.
- FOV override via `CameraAPI::tryGetFOV` hook confirmed working
  on-device.
- Single-finger hold+drag zoom confirmed working on-device.
