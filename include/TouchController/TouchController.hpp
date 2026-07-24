#pragma once
// TouchController/TouchController.hpp
//
// Turns raw pl::input::TouchEvent's into ZoomController's three calls
// (BeginZoom/UpdateDrag/EndZoom), tracking Android's pointerId so the
// same finger that started a hold is the one whose movement counts -
// see docs/architecture.md for why (an earlier ButtonInfo-based
// approach couldn't achieve single-finger drag because the widget
// captured its own pointer's move events internally).
//
// STEP 2 SCOPE: there's no ZoomButton yet, so this version starts a
// hold on ANY touch, anywhere on screen - not gated behind a hit-test
// zone. This is intentional and temporary, purely to verify
// TouchController correctly drives ZoomController before ZoomButton
// (Step 3) adds the real zone + Contains() check. Expect this to be
// disruptive to normal play while testing (every touch anywhere starts
// a zoom) - that's expected for this step, not a bug.

namespace touch_controller {

// Registers the touch callback. Call once, from mod load().
void Install();

} // namespace touch_controller
