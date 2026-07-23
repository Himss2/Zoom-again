#pragma once
// Zoom/ZoomState.hpp
//
// Shared state for the zoom feature, read/written from three different
// modules:
//   - Input  (writes: begins a hold, updates factor from drag, begins release)
//   - Camera (reads: whether to override FOV this frame, and by how much)
//   - UI     (reads: whether to draw the zone as "active")
//
// Kept as plain atomics behind free functions rather than a class with
// getters/setters spread across headers, since every caller only ever
// wants one field at a time.

#include <atomic>
#include <cstdint>

namespace zoom {

constexpr float kNeutralZoom = 1.0f;
constexpr float kMinZoom     = 0.10f;
constexpr float kMaxZoom     = 4.00f;

// True while the zoom zone is held, or still animating back to neutral
// after release.
bool IsActive();
void SetActive(bool active);

// True once released, until the factor has animated back to neutral.
bool IsReleasing();
void SetReleasing(bool releasing);

// Current zoom factor (divisor applied to base FOV - see Camera/FovOverride.hpp).
float GetFactor();
void SetFactor(float factor);
void AdjustFactor(float delta); // clamps to [kMinZoom, kMaxZoom]

// The Android pointerId currently "owning" the zoom zone, or -1 if none.
int GetOwnerPointerId();
void SetOwnerPointerId(int pointerId);

} // namespace zoom
