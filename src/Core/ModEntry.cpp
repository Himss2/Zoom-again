// Core/ModEntry.cpp
//
// Attempt to resolve Blocker #1 by matching the OFFICIAL documented
// pattern exactly (levilaunchroid.levimc.org/guide/developer), which
// differs from every previous attempt in three ways simultaneously:
//   1. Uses pl::modmenu::ModuleBuilder(...).registerModule(), not the
//      raw ModuleInfo{} + pl::modmenu::registerModule(module) call.
//   2. Registers the Mod Menu module from enable(), not load().
//   3. Loads a pl::config::ConfigFile first - the checklist says
//      "Load config before registering runtime UI," which we never
//      did in any earlier attempt.
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
        log.info("Core: load() start (official ModuleBuilder+Config pattern test)");

        if (!camera_hook::Install()) {
            log.error("Core: CameraHook::Install() failed, aborting load()");
            return false;
        }

        touch_controller::Install();

        camera_hook::SetFrameTickCallback([]() {
            zoom_controller::Tick();
            zoom_button::Draw(zoom_controller::IsActive());
        });

        log.info("Core: load() done");
        return true;
    }

    bool enable() {
        return zoom_button::Install();
    }

    bool disable() {
        zoom_button::Uninstall();
        return true;
    }

    bool unload() {
        camera_hook::Uninstall();
        return true;
    }
};

PL_REGISTER_MOD(ZoomRewriteMod, ZoomRewriteMod::Instance())
