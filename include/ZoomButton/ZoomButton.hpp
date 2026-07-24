#pragma once
// ZoomButton/ZoomButton.hpp
//
// Hand-draws the "ZM" zone every frame via pl::modmenu::submitDrawCommands
// - NOT a pl::modmenu::ButtonInfo widget, because ButtonInfo captures
// its own pointer's move events internally, which broke single-finger
// hold+drag in the previous project (see docs/research-notes.md).
// Owns the one pl::modmenu::registerModule() call needed to appear in
// the Mod Menu at all (a module id is required even with no
// ButtonInfo attached).
//
// THIS IS WHERE BLOCKER #1 LIVES (see docs/architecture.md): every
// previous call to registerModule() crashed deterministically
// (SIGSEGV, fault addr 0x10), even in a maximally minimal test, with
// only ZoomRewrite/SmoothZoom active and no other native mods loaded
// concurrently. If it crashes again here under that exact
// condition, this is very likely a genuine bug in the SDK/launcher
// itself, worth reporting upstream with the crash tombstone as
// evidence - not something fixable from mod code.

namespace zoom_button {

// Registers the ModMenu module. Call once, from mod load(). THE
// BLOCKER #1 CALL - see file header.
void Install();

// Unregisters the module. Call from mod unload().
void Uninstall();

// Redraws the zone (color reflects isActive). Call once per rendered
// frame - Core wires this to CameraHook's per-frame tick.
void Draw(bool isActive);

// True if the given screen-space point falls inside the zone - used by
// TouchController to decide whether an ACTION_DOWN starts a hold.
bool Contains(float x, float y);

} // namespace zoom_button
