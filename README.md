# ZoomRewrite

Flarial-style single-finger hold-and-drag zoom (drag up = zoom in, drag
down = zoom out) with a smooth release animation, for LeviLaunchroid /
Minecraft Bedrock 1.26.33.1 (arm64-v8a).

Created by **Himss**.

## Architecture

```
ZoomRewrite/
├── assets/manifest.json          - mod manifest
├── include/ , src/
│   ├── Core/              - mod lifecycle glue (ModContext, Config), shared logger/mod-id access
│   ├── CameraHook/        - CameraAPI::tryGetFOV hook (the actual zoom effect) + Options::getHideHand hook
│   ├── TouchController/   - raw single-finger touch tracking (press/drag/release), pointerId-aware
│   ├── ZoomController/    - zoom state machine (idle/holding/releasing) + release animation
│   └── ZoomButton/        - hand-drawn "ZM" zone (rounded rect) + hit-testing
├── docs/                  - architecture.md, research-notes.md
└── .github/workflows/build.yml
```

Dependency direction: `Core` ties everything together via `ModEntry`.
`TouchController` -> `ZoomController` -> `CameraHook`. `ZoomButton` is
read by `TouchController` (`Contains()`) and drawn once per frame via
`CameraHook`'s frame-tick callback (wired up in `Core::ModEntry`, the
one intentional cross-module call outside the dependency chain).

## Status

| Piece | Status |
|---|---|
| Project structure, build | Done |
| FOV hook (`CameraAPI::tryGetFOV`) | **Confirmed working on-device** |
| Single-finger drag tracking | **Confirmed working on-device** |
| Release animation | Implemented, tunable via Mod Menu (`zoomAnimSpeed`) |
| ModMenu module registration | **Working** - see "Fixed" note below for how this was resolved |
| Config (speed, opacity, position, scale, hide-hand toggle) | **Working**, persisted via `pl::config::ConfigFile` |
| Button UI | Rounded-rect style (outline + fill + text), replacing the earlier pixel-notch look - not yet confirmed smooth on-device at all corner-step counts |
| `hideHandOnZoom` toggle | **Known bug - does not visibly hide the hand.** See "Open problem" below |

## Fixed: SIGSEGV in `pl::modmenu::registerModule`

An earlier version of this project crashed consistently with a SIGSEGV
inside `pl::modmenu::registerModule` (fault addr `0x10`), even in a
maximally minimal build. Root cause: **`CameraHook::Install()` (which
hooks `CameraAPI::tryGetFOV`) was being installed before the launcher's
own Inbuilt Zoom Mod finished its `nativeInit()`**, causing a vtable
collision. Fix, now in `Core/ModEntry.cpp`:

- `camera_hook::Install()` is called from **`enable()`**, not `load()`,
  so it runs after Inbuilt Mods have finished initializing.
- ModMenu module registration (`config::RegisterModMenu()`) also runs
  from `enable()`.

(An earlier theory blamed a race condition between mods loading
concurrently on the launcher's thread pool - that was never confirmed
and is superseded by the above.)

## Open problem: `hideHandOnZoom` toggle doesn't work

The toggle is stored and persisted correctly, but has no visible
effect in-game. Root cause: `CameraHook::Install()` resolves
`Options::getHideHand` by **guessing** across a list of candidate
vtable slots (`{27, 28, 29, 26, 25, 30, 31, 32, 23, 22, 24}`) and
stops at the first one that resolves and hooks successfully -
**without verifying it's actually `getHideHand`**. Unlike
`CameraAPI::tryGetFOV` (confirmed via DWARF analysis - see
`docs/research-notes.md`), this slot was never independently verified,
so the hook is very likely attached to the wrong function.

Next step: add per-slot logging during resolution, then a passthrough
diagnostic hook to compare call frequency against real hand-hiding
game actions (bow draw, shield block) to identify the correct slot -
or find the same DWARF-confirmable evidence used for `tryGetFOV`.

## Build

Push to `main` - GitHub Actions builds it and publishes
`ZoomRewrite.levipack` to the repo's "latest" release automatically.

## Confirmed technical facts (safe to reuse without re-deriving)

```cpp
pl::memory::resolveVtableFunction("9CameraAPI", 7, "libminecraftpe.so")
// -> CameraAPI::tryGetFOV, hook signature: uint64_t(void* thisPtr)
```

Recovered via DWARF `GNU_call_site_value` entries in LeviLauncher's own
built-in Zoom mod (`libinbuiltmods.so`, ships unstripped in the launcher
APK) - not guessed.

FOV value encoding (confirmed empirically on-device against bow-draw,
spyglass, and BedrockTools' own zoom): the `uint64_t` splits into two
32-bit halves - high 32 bits = 1 if an override is active, low 32 bits =
the override value as a `float` bit-pattern. The float acts as a divisor
on the base FOV: larger = narrower FOV (zoomed in), smaller = wider FOV
(zoomed out beyond normal).

`Options::getHideHand`'s vtable slot is **not yet confirmed** - see
"Open problem" above. Do not treat the current candidate-slot list as
a confirmed fact.
