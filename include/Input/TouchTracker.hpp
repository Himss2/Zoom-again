#pragma once
// Input/TouchTracker.hpp
//
// Tracks a single finger's press-hold-drag-release cycle directly via
// pl::input::registerTouchCallback, keyed on Android's pointerId so the
// same finger is followed start to finish.
//
// This exists because an earlier version used pl::modmenu::ButtonInfo
// for the zoom control and tried to read drag movement via its
// onEvent's ButtonEvent::Scroll case - CONFIRMED ON-DEVICE that this
// never fires for touch-drag-while-held (only Down/Up did). The
// ButtonInfo widget captures its own pointer's subsequent move events
// internally, so they never reach a separate global touch callback for
// that same finger. Bypassing ButtonInfo for the zoom control entirely
// (see UI/ZoomZone.hpp, which hand-draws the zone instead) and handling
// 100% of the touch stream ourselves is what makes single-finger
// hold+drag work.

namespace input {

// Registers the global touch callback. Call once, from mod load().
void InstallTouchTracking();

} // namespace input
