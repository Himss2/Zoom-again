# Research notes

Background on how the confirmed facts in the main README were actually
derived - kept here so the reasoning isn't lost, without cluttering the
top-level README.

## Finding the FOV hook target

LeviLauncher's own built-in Zoom mod ships as `libinbuiltmods.so` inside
the launcher APK, and unlike almost every third-party mod `.so`
inspected during this project, it ships **unstripped** - full DWARF
debug info included.

Its symbol table alone showed a lot: `CameraAPI_tryGetFOV_hook(void*)`,
`VanillaCameraAPI_getPlayerViewPerspectiveOption_hook(void*)`, plus
JNI-facing functions (`nativeOnKeyDown`, `nativeOnScroll`,
`nativeSetZoomLevel`, etc.) confirming the mod bridges Java-side
input/UI to a native FOV hook.

The genuinely load-bearing find was in the DWARF `GNU_call_site_value`
entries for the inlined `findAndHookCameraAPI()` function - these record
the *actual compiled constant arguments* passed to
`pl::memory::resolveVtableFunction()` at its call site, recovered via:

```
readelf --debug-dump=info libinbuiltmods.so | grep -A 200 \
  "abstract_origin: <0x1f1e>"   # (address of the inlined function)
```

This showed register values `x1=10`, `x2=7`, `x4=17` at the call site -
which line up exactly with:
- `x1` = length of the string `"9CameraAPI"` (10 chars)
- `x2` = the `slot` argument = **7**
- `x4` = length of the string `"libminecraftpe.so"` (17 chars)

i.e. the exact call is:
```cpp
pl::memory::resolveVtableFunction("9CameraAPI", 7, "libminecraftpe.so")
```
The same technique on the second hook (`VanillaCameraAPI`) gave
`x1=18` (length of `"16VanillaCameraAPI"`) and `x2=7` again.

## Confirming the FOV value encoding

A passthrough-only diagnostic hook (calls the original function, logs
the raw `uint64_t` several different ways, returns it unchanged - safe,
since it can't alter behavior) was used to observe real values during:
bow draw, spyglass use, and BedrockTools' own zoom at two different
zoom depths. Splitting the `uint64_t` into two 32-bit halves and
reinterpreting the low half as a `float` consistently matched expected
behavior (e.g. BedrockTools zoomed in at `1.71`, zoomed out at `0.08`),
confirming: high 32 bits = "has an override" flag, low 32 bits = the
override value, acting as a divisor on the base FOV.

## Why ButtonInfo couldn't be used for single-finger drag

An earlier version used `pl::modmenu::ButtonInfo` with
`ButtonBehavior::Hold` for the zoom control, and tried to read drag
movement via its `onEvent` callback's `ButtonEvent::Scroll` case.
Confirmed on-device via logcat: only `Down`/`Up` ever fired - `Scroll`
never appeared even while actively dragging. A separate
`pl::input::registerTouchCallback` was then added to track movement
independently, which worked, but only for a *second*, different finger
- the finger actually pressing the `ButtonInfo` widget never generated
move events reaching the callback (the widget captures its own
pointer's stream internally). Dropping `ButtonInfo` for this control
entirely - hand-drawing the zone via `submitDrawCommands` and handling
100% of the touch stream ourselves, keyed on Android's `pointerId` -
is what made true single-finger hold+drag work.

## Reference material not directly used

Analyzed the Flarial client (`libflarialclient.so`) for comparison.
It's not built on the `preloader-android` SDK this project uses (no
`pl::` imports at all - it has its own loader/hook stack), and ships
fully stripped (no DWARF), so none of its call parameters were directly
reusable. Still surfaced real, useful Minecraft-internal names in case
the `CameraAPI::tryGetFOV` route ever needs a fallback:
- `LevelRendererPlayer::getFov` - a higher-level FOV getter, likely
  called once per rendered frame
- `ViewFovEvent`, dispatched through Minecraft's own `entt` (EnTT) ECS
  event system - Flarial listens to this rather than hooking a vtable
  slot directly
