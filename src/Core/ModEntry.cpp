// Core/ModEntry.cpp
//
// STEP 1 of the build order in docs/architecture.md: CameraHook +
// ZoomController only. Deliberately no pl::modmenu::registerModule call
// yet, no TouchController, no ZoomButton - this isolates "does the FOV
// hook still work" from everything else, including Blocker #1 (the
// unresolved registerModule SIGSEGV), since skipping registerModule
// entirely sidesteps it for this phase.
//
// TEMPORARY test wiring: BeginZoom() + a fixed UpdateDrag() are called
// once in load() purely to give a visible, no-input-required way to
// confirm the hook still works on-device. Remove this block once
// Step 2 (TouchController) exists to drive it for real.

#include "Core/ModContext.hpp"
#include "CameraHook/CameraHook.hpp"
#include "ZoomController/ZoomController.hpp"

#include <pl/Mod.hpp>

class ZoomRewriteMod {
public:
    static ZoomRewriteMod& Instance() {
        static ZoomRewriteMod inst;
        return inst;
    }

    bool load() {
        auto& log = core::Log();
        log.info("Core: load() start (Step 1: CameraHook + ZoomController only)");

        if (!camera_hook::Install()) {
            log.error("Core: CameraHook::Install() failed, aborting load()");
            return false;
        }
        zoom_controller::Install();

        // --- TEMPORARY: remove once TouchController exists (Step 2) ---
        // CORRECTED direction: on-device testing showed factor=1.7 looks
        // WIDE (zoomed out), not zoomed in - the value is a MULTIPLIER on
        // base FOV, not a divisor as first assumed. Smaller = zoomed in.
        // This also explains the old single-file project's "drag down
        // zooms in" bug: the drag mechanics weren't backwards, the
        // factor's meaning was.
        zoom_controller::BeginZoom();
        zoom_controller::UpdateDrag(-0.5f); // expect a visible, fixed zoom-in (factor -> 0.5)
        log.info("Core: test zoom engaged (factor should read ~0.50) - "
                 "confirm visually, then remove this block for Step 2");
        // --- end temporary block ---

        log.info("Core: load() done");
        return true;
    }

    bool enable() { return true; }
    bool disable() { return true; }

    bool unload() {
        camera_hook::Uninstall();
        return true;
    }
};

PL_REGISTER_MOD(ZoomRewriteMod, ZoomRewriteMod::Instance())
