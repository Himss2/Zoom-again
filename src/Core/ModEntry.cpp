// Core/ModEntry.cpp
//
// STEP 3 (final) of the build order in docs/architecture.md:
// CameraHook + ZoomController + TouchController + ZoomButton.
//
// This is where Blocker #1 gets tested for real: ZoomButton::Install()
// makes the one pl::modmenu::registerModule() call that crashed
// deterministically in every previous attempt. If it still crashes
// here - with ONLY this mod active, no other native mods loaded
// concurrently - that's strong evidence of a genuine SDK/launcher bug,
// not something in this project's code. See docs/architecture.md.
//
// core::Init() MUST be the very first call in load() - see
// Core/ModContext.hpp for why.

#include "Core/ModContext.hpp"
#include "CameraHook/CameraHook.hpp"
#include "ZoomController/ZoomController.hpp"
#include "TouchController/TouchController.hpp"
#include "ZoomButton/ZoomButton.hpp"

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
        log.info("Core: load() start (Step 3: + ZoomButton, Blocker #1 test)");

        if (!camera_hook::Install()) {
            log.error("Core: CameraHook::Install() failed, aborting load()");
            return false;
        }

        zoom_button::Install(); // <-- the Blocker #1 call
        touch_controller::Install();

        camera_hook::SetFrameTickCallback([]() {
            zoom_controller::Tick();
            zoom_button::Draw(zoom_controller::IsActive());
        });

        log.info("Core: load() done");
        return true;
    }

    bool enable() { return true; }
    bool disable() { return true; }

    bool unload() {
        camera_hook::Uninstall();
        zoom_button::Uninstall();
        return true;
    }
};

PL_REGISTER_MOD(ZoomRewriteMod, ZoomRewriteMod::Instance())
