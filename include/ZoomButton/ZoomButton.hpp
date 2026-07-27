#pragma once
// ZoomButton/ZoomButton.hpp
//
// Hand-draws the "ZM" zone every frame via pl::modmenu::submitDrawCommands
// - NOT a pl::modmenu::ButtonInfo widget, because ButtonInfo captures
// its own pointer's move events internally, which broke single-finger
// hold+drag in the previous project (see docs/research-notes.md).
//
// Registration now follows the OFFICIAL documented pattern
// (levilaunchroid.levimc.org/guide/developer): uses
// pl::modmenu::ModuleBuilder(...).registerModule() (not the raw
// ModuleInfo{} + registerModule(module) call every earlier crashing
// attempt used), loads a pl::config::ConfigFile first per the
// checklist ("Load config before registering runtime UI"), and is
// called from enable() rather than load() - see Core/ModEntry.cpp.

namespace zoom_button {

// Loads config and registers the ModMenu module via ModuleBuilder.
// Returns false on failure. Call once, from mod enable().
bool Install();

// Unregisters the module. Call from mod disable().
void Uninstall();

// Redraws the zone (color reflects isActive). Call once per rendered
// frame - Core wires this to CameraHook's per-frame tick.
void Draw(bool isActive);

// True if the given screen-space point falls inside the zone - used by
// TouchController to decide whether an ACTION_DOWN starts a hold.
bool Contains(float x, float y);

// --- HELPER UNTUK POSITION & SCALE DINAMIS ---
float GetX();
float GetY();
float GetScale();
void SetPosition(float x, float y);
void SetScale(float scale);

} // namespace zoom_button
