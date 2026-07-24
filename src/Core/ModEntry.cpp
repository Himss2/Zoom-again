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
        zoom_controller::BeginZoom();
        zoom_controller::UpdateDrag(0.7f); // expect a visible, fixed zoom-in
        log.info("Core: test zoom engaged (factor should read ~1.70) - "
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
