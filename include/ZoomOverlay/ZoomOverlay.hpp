#pragma once
// ZoomOverlay/ZoomOverlay.hpp
//
// Renders the "ZM" zone directly via raw OpenGL ES calls, hooked into
// eglSwapBuffers - completely independent of pl::modmenu, and therefore
// of Blocker #1 (the still-unresolved pl::modmenu::registerModule
// crash - see docs/architecture.md). This is the fallback path chosen
// specifically to sidestep that blocker rather than wait for it.
//
// Hook target: eglSwapBuffers is a standard, public EGL API function
// exported by libEGL.so on every Android device - resolved via
// dlopen+dlsym, NOT vtable/RTTI resolution. This is more reliable than
// the CameraAPI::tryGetFOV hook (which needed RTTI-based
// resolveVtableFunction) since it's a stable public C ABI, not an
// internal game class.

namespace zoom_overlay {

// Hooks eglSwapBuffers and prepares GL resources (shader program) for
// rendering. Call once, from mod load().
bool Install();

// Reverts the hook. Call from mod unload().
void Uninstall();

// True if the given screen-space point (pixels) falls inside the
// zone - used by TouchController to decide whether an ACTION_DOWN
// starts a hold.
bool Contains(float x, float y);

} // namespace zoom_overlay
