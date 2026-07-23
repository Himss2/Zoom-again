# SmoothZoom

Flarial-style single-finger hold-and-drag zoom (drag up = zoom in, drag
down = zoom out) with a smooth release animation, for LeviLaunchroid /
Minecraft Bedrock 1.26.33.1 (arm64-v8a).

Created by **Himss**.

## Architecture

```
Zoom/
├── assets/manifest.json      - mod manifest
├── include/ , src/
│   ├── Core/     - mod lifecycle glue, shared logger/mod-id access
│   ├── Camera/   - CameraAPI::tryGetFOV hook (the actual zoom effect)
│   ├── Input/    - raw single-finger touch tracking (press/drag/release)
│   ├── UI/       - hand-drawn "ZM" zone + ModMenu module registration
│   └── Zoom/     - shared state (active/releasing/factor/owner pointer)
└── .github/workflows/build.yml
```

Dependency direction: `Core` ties everything together; `Camera`, `Input`,
and `UI` each depend on `Zoom` (shared state) and `Core` (logging), but
not on each other directly - `Camera::Detour` calls `ui::DrawZoomZone()`
once per frame since that hook is a convenient per-frame tick, which is
the one intentional cross-module call outside of `Core::ModEntry`.

## Status

| Piece | Status |
|---|---|
| Project structure, build | Done |
| FOV hook (`CameraAPI::tryGetFOV`) | **Confirmed working on-device** - holding the zone visibly zooms |
| Single-finger drag tracking | **Confirmed working on-device** - logcat showed `deltaY=... -> factor=...` while dragging the same finger that started the hold |
| Release animation | Implemented, not yet independently confirmed smooth on-device |
| ModMenu module registration | **Currently causing a SIGSEGV - see below** |

## Open problem: SIGSEGV in `pl::modmenu::registerModule`

Every build so far - including a maximally minimal one that does
*nothing* except construct a `ModuleInfo` and call `registerModule()` -
crashes with the exact same signature:

```
signal 11 (SIGSEGV), fault addr 0x10
#00 pl::modmenu::registerModule(ModuleInfo const&)+1476  (libpreloader.so)
#01 <our load function>                                   (libSmoothZoom.so)
```

What's been ruled out so far:
- **Not bad data**: the embedded logcat inside the crash tombstone shows
  our own log line printing valid, non-empty strings
  (`moduleId='smoothzoom.module' displayName='Smooth Zoom'
  modId='SmoothZoom'`) immediately before the crash.
- **Not the `hideInHudEditor` field**: removed it, crash persisted at the
  identical offset.
- **Not "only active ModMenu mod"** (the known OffhandFix quirk, fixed
  there by registering in `load()` instead of `enable()` - already done
  here too): crashed both with BedrockTools active and with it disabled.
- **Not an environment-wide regression**: the older OffhandFix project,
  re-tested during this same debugging session, still works fine.
- **A `ModuleInfo` constructed identically in OffhandFix's `load()`
  works without crashing.**

What's NOT yet confirmed, and is the next thing to check:
- **Whether some *other* native mod was loading concurrently.** Mods
  load on a background thread pool (`pool-32-thread-*` in every crash's
  thread name) - if `registerModule`'s internal registry isn't
  thread-safe, two mods registering at nearly the same moment could
  race. This was proposed but never cleanly isolated - the "without
  BedrockTools" test may still have had OffhandFix enabled
  simultaneously. **To test properly: disable every other mod
  (OffhandFix included), leave only SmoothZoom enabled, and retry.**

## Build

Push to `main` - GitHub Actions builds it and publishes
`SmoothZoom.levipack` to the repo's "latest" release automatically.

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
