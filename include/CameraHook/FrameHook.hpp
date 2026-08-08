#pragma once
// CameraHook/FrameHook.hpp
//
// Hooks eglSwapBuffers directly so we get exactly one callback per
// actually-rendered frame - independent of how often any other hooked
// function (like CameraAPI::tryGetFOV) happens to be called. Replaces
// the previous approach of stealing a tick from inside DetourFOV,
// which caused the choppy release animation (tryGetFOV's call
// frequency isn't a reliable per-frame signal).

#include <functional>

namespace frame_hook {

bool Install();
void Uninstall();

// Registers the callback invoked once per rendered frame.
void SetFrameCallback(std::function<void()> callback);

} // namespace frame_hook
