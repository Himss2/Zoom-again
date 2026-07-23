#pragma once
// UI/ZoomZone.hpp
//
// Hand-draws the "ZM" zone every frame via pl::modmenu::submitDrawCommands
// instead of using a pl::modmenu::ButtonInfo widget - see
// Input/TouchTracker.hpp for why. Also owns the one
// pl::modmenu::registerModule() call needed to appear in the Mod Menu at
// all (a module id is required even with no button attached).

namespace ui {

// Registers the ModMenu module. Call once, from mod load().
void RegisterZoomModule();

// Unregisters the module. Call from mod unload().
void UnregisterZoomModule();

// Redraws the zone (color reflects whether it's currently held). Cheap
// enough to call once per rendered frame - see Camera/FovOverride.cpp,
// which piggybacks this on its per-frame hook.
void DrawZoomZone();

// True if the given screen-space point falls inside the zone - used by
// Input/TouchTracker.cpp to decide whether an ACTION_DOWN starts a hold.
bool PointInZoomZone(float x, float y);

} // namespace ui
