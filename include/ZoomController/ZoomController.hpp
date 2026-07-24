#pragma once
// ZoomController/ZoomController.hpp
//
// Owns the zoom state machine (idle / holding / releasing) and the
// release animation curve. The only module allowed to call CameraHook.
// Knows nothing about touch coordinates, pointer ids, or drawing - see
// docs/architecture.md for the full boundary rules.

namespace zoom_controller {

constexpr float kNeutralFactor = 1.0f;
constexpr float kMinFactor     = 0.10f;
constexpr float kMaxFactor     = 4.00f;


// Starts a hold: resets to neutral and begins overriding FOV.
void BeginZoom();

// Adjusts the current factor by `delta` (arbitrary unit - e.g. pixels
// of drag times a caller-chosen sensitivity), clamped to
// [kMinFactor, kMaxFactor]. No-op if a zoom isn't currently active.
void UpdateDrag(float delta);

// Begins animating back to neutral; CameraHook's override is cleared
// automatically once the animation finishes (see Tick()).
void EndZoom();

// Must be called once per frame (wired to CameraHook's frame tick by
// Core) - advances the release animation.
void Tick();

// For ZoomButton to decide what color to draw itself.
bool IsActive();

} // namespace zoom_controller
