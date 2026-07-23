# ZoomRewrite — Architecture

## Purpose

A clean rebuild of the SmoothZoom/Zoom-for-Levi feature (Flarial-style
hold-and-drag zoom for Minecraft Bedrock via LeviLaunchroid), with
strict separation of concerns so future features (sensitivity config,
button repositioning, alternate input methods) don't require touching
the core logic.

## Modules and their contracts

### `CameraHook`
**Knows:** how to apply an FOV override to the game, and how to hand
control back to vanilla.
**Does NOT know:** anything about touch, drag, buttons, or animation
curves. It exposes something like:
```
void CameraHook::SetOverride(float factor); // divisor on base FOV
void CameraHook::ClearOverride();
```
and internally owns the `pl::memory::hook` install/uninstall and the
`uint64_t` packing/unpacking. Nothing outside this module should ever
touch the raw hook return value or vtable resolution directly.

### `ZoomController`
**Knows:** the zoom state machine - idle / holding / releasing - and
the animation curve back to neutral. This is the only module with
"business logic" in it.
**Does NOT know:** touch coordinates, pointer ids, or drawing. It
exposes something like:
```
void ZoomController::BeginZoom();
void ZoomController::UpdateDrag(float delta);   // arbitrary unit, e.g. pixels
void ZoomController::EndZoom();
void ZoomController::Tick();                    // called once per frame; drives release animation, pushes current factor to CameraHook
```
`ZoomController` is the one module allowed to call into `CameraHook`.

### `TouchController`
**Knows:** how to turn raw `pl::input::TouchEvent`s into the three
calls above, including pointer-id tracking so the same finger that
started a hold is the one whose movement counts, and rejecting/ignoring
events from other fingers.
**Does NOT know:** anything about zoom factors, FOV, or animation - it
never reads or writes `ZoomController`'s internal state, only calls its
three public methods.

### `ZoomButton`
**Knows:** how to draw the on-screen zone (`submitDrawCommands`) and
what rectangle counts as "inside" for hit-testing.
**Does NOT know:** touch handling. It does not register any input
callback itself and does not consume touch events - `TouchController`
is the only module that talks to `pl::input`. `ZoomButton` exposes
something like:
```
bool ZoomButton::Contains(float x, float y);
void ZoomButton::Draw(bool isActive);
```
`TouchController` calls `Contains()` to decide whether an ACTION_DOWN
starts a hold; `ZoomController::Tick()` (or `CameraHook`'s per-frame
hook, as a convenient tick source) calls `Draw()`.

## Dependency direction (enforced, not just convention)

```
TouchController ──> ZoomController ──> CameraHook
ZoomButton      ──> (nothing)     <── read by TouchController (Contains) and Tick (Draw)
```

No module other than `ZoomController` may call `CameraHook`. No module
other than `TouchController` may touch `pl::input`. If a change requires
violating this, it's a sign the boundary is in the wrong place - fix the
boundary, don't punch through it.

## Build order (bisect-friendly, don't skip steps)

1. **`CameraHook` + `ZoomController` alone.** No input, no UI. Hardcode
   a test path (e.g. `ZoomController::BeginZoom()` called once from
   `load()`, `UpdateDrag()` fed a fixed fake value) and confirm FOV
   visibly changes on-device. This isolates "does the hook still work"
   from everything else.
2. **Add `TouchController`.** Still no `ZoomButton` - trigger
   `BeginZoom()`/`EndZoom()` some other way (a key callback is fine
   temporarily) and confirm drag tracking drives the same visible zoom
   via logcat, before any drawing exists.
3. **Add `ZoomButton` last.** Purely visual; if 1 and 2 already work,
   this step can't reintroduce a logic bug, only a drawing/hit-testing
   one.

## Confirmed technical facts to carry over as-is (do not re-derive)

These cost real reverse-engineering effort (DWARF analysis of
LeviLauncher's own built-in Zoom mod) and should go directly into
`CameraHook` rather than being rediscovered:

```cpp
pl::memory::resolveVtableFunction("9CameraAPI", 7, "libminecraftpe.so")
// -> CameraAPI::tryGetFOV, hook signature: uint64_t(void* thisPtr)
```

FOV value encoding: the `uint64_t` splits into two 32-bit halves - high
= 1 if an override is active, low = the override value as a `float`
bit-pattern, acting as a divisor on the base FOV (larger = zoomed in,
smaller = zoomed out). Confirmed empirically on-device against
bow-draw, spyglass, and BedrockTools' own zoom.

Full derivation history: see the old project's `docs/research-notes.md`
- copy it into this repo's `docs/` rather than re-running the DWARF
  analysis from scratch.

## Blocker #1 — must be resolved before new features, not after

Every build of the previous project crashed with an identical SIGSEGV
inside `pl::modmenu::registerModule` (fault addr `0x10`), including a
maximally minimal build that did nothing but construct a `ModuleInfo`
and call `registerModule()`. Ruled out so far: bad/empty data (logcat
embedded in the crash tombstone showed valid, non-empty strings going
in), the `hideInHudEditor` field, and an environment-wide regression
(the older OffhandFix project still works fine, tested in the same
session).

**Still unanswered, and the next concrete thing to check:** whether
another native mod (OffhandFix specifically) was also enabled during
the "crashes even without BedrockTools" test. Mods load on a background
thread pool - if two mods' `load()` calls race inside the launcher's
internal ModMenu registry, that would explain a crash that doesn't
depend on BedrockTools specifically. Test with **only** ZoomRewrite
enabled and every other native mod (OffhandFix included) disabled
before writing any new `ZoomButton`/`ZoomController` code that also
calls `registerModule` - if it crashes even then, this is a genuine SDK
bug worth reporting upstream with the tombstone as evidence, not
something fixable from mod code.
