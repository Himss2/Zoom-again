// Core/ModEntry.cpp
//
// STEP 2 of the build order in docs/architecture.md: CameraHook +
// ZoomController + TouchController. Still no ZoomButton and no
// pl::modmenu::registerModule call (Blocker #1 remains unresolved -
// see docs/architecture.md) - purely testing that touch-driven drag
// correctly drives the zoom, via logcat and visual confirmation, with
// zero UI drawn yet.
//
// Step 1's fixed-value test block (BeginZoom() + a hardcoded
// UpdateDrag()) has been removed now that TouchController exists to
// drive it for real.

#include "Core/ModContext.hpp"
#include "CameraHook/CameraHook.hpp"
#include "ZoomController/ZoomController.hpp"
#include "TouchController/TouchController.hpp"

#include <pl/Mod.hpp>

class ZoomRewriteMod {
public:
    static ZoomRewriteMod& Instance() {
        static ZoomRewriteMod inst;
        return inst;
    }

    bool load() {
        auto& log = core::Log();
        log.info("Core: load() start (Step 2: + TouchController)");

        if (!camera_hook::Install()) {
            log.error("Core: CameraHook::Install() failed, aborting load()");
            return false;
        }
        zoom_controller::Install();
        touch_controller::Install();

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
