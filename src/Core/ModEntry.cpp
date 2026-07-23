// Core/ModEntry.cpp
//
// Lifecycle glue: PL_REGISTER_MOD lives here, calling into each
// module's own Install/Register functions in a fixed order:
//   UI::RegisterZoomModule()   - must happen first, registers with ModMenu
//   Input::InstallTouchTracking()
//   Camera::InstallFovHook()

#include "Core/ModContext.hpp"
#include "Camera/FovOverride.hpp"
#include "Input/TouchTracker.hpp"
#include "UI/ZoomZone.hpp"

#include <pl/Mod.hpp>

class SmoothZoomMod {
public:
    static SmoothZoomMod& Instance() {
        static SmoothZoomMod inst;
        return inst;
    }

    bool load() {
        core::Log().info("Core: load() start");
        ui::RegisterZoomModule();
        input::InstallTouchTracking();
        camera::InstallFovHook();
        core::Log().info("Core: load() done");
        return true;
    }

    bool enable() {
        return true;
    }

    bool disable() {
        return true;
    }

    bool unload() {
        camera::UninstallFovHook();
        ui::UnregisterZoomModule();
        return true;
    }
};

PL_REGISTER_MOD(SmoothZoomMod, SmoothZoomMod::Instance())
