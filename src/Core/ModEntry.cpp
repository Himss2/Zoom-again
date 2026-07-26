// Core/ModEntry.cpp
//
// FALLBACK PATH: sidesteps Blocker #1 (the still-unresolved
// pl::modmenu::registerModule crash - see docs/architecture.md)
// entirely by not calling any pl::modmenu function at all.
// ZoomOverlay renders the zone via a raw eglSwapBuffers hook instead
// of ZoomButton's pl::modmenu::submitDrawCommands, so there's no
// Mod Menu presence for now - just a self-drawn overlay.
//
// core::Init() MUST be the very first call in load() - see
// Core/ModContext.hpp for why.

#include "Core/ModContext.hpp"
#include "CameraHook/CameraHook.hpp"
#include "ZoomController/ZoomController.hpp"
#include "TouchController/TouchController.hpp"
#include "ZoomOverlay/ZoomOverlay.hpp"

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
        log.info("Core: load() start (fallback: ZoomOverlay, no ModMenu)");

        if (!camera_hook::Install()) {
            log.error("Core: CameraHook::Install() failed, aborting load()");
            return false;
        }

        if (!zoom_overlay::Install()) {
            log.error("Core: ZoomOverlay::Install() failed, aborting load()");
            return false;
        }

        touch_controller::Install();

        camera_hook::SetFrameTickCallback([]() {
            zoom_controller::Tick();
        });

        log.info("Core: load() done");
        return true;
    }

    bool enable() { return true; }
    bool disable() { return true; }

    bool unload() {
        camera_hook::Uninstall();
        zoom_overlay::Uninstall();
        return true;
    }
};

PL_REGISTER_MOD(ZoomRewriteMod, ZoomRewriteMod::Instance())
