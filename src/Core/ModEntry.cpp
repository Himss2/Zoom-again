// Core/ModEntry.cpp
//
// STEP 2 of the build order in docs/architecture.md: CameraHook +
// ZoomController + TouchController. Still no ZoomButton and no
// pl::modmenu::registerModule call (Blocker #1 remains unresolved -
// see docs/architecture.md).
//
// core::Init() MUST be the very first call in load() - see
// Core/ModContext.hpp for why (pl::mod::NativeMod::current() crashes
// if called later from an async callback like TouchController's touch
// handler; Init() caches it once while it's still valid).

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
        core::Init(); // MUST be first - see Core/ModContext.hpp

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
