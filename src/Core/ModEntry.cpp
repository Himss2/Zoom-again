#include "Core/ModContext.hpp"
#include "Core/Config.hpp"
#include "CameraHook/CameraHook.hpp"
#include "CameraHook/FrameHook.hpp"
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
        core::Init();

        auto& log = core::Log();
        log.info("Core: load() start");

        config::Load();
        touch_controller::Install();

        // FrameHook is independent of CameraAPI/Options vtables, so it's
        // safe to install here (no collision risk with Inbuilt Zoom Mod).
        if (!frame_hook::Install()) {
            log.error("Core: FrameHook::Install() failed");
            return false;
        }

        frame_hook::SetFrameCallback([]() {
            zoom_controller::Tick();
            zoom_button::Draw(zoom_controller::IsActive());
        });

        log.info("Core: load() done");
        return true;
    }

    bool enable() {
        auto& log = core::Log();

        config::RegisterModMenu();

        if (!camera_hook::Install()) {
            log.error("Core: CameraHook::Install() failed during enable()");
            return false;
        }

        return zoom_button::Install();
    }

    bool disable() {
        camera_hook::Uninstall();
        zoom_button::Uninstall();
        return true;
    }

    bool unload() {
        camera_hook::Uninstall();
        frame_hook::Uninstall();
        return true;
    }
};

PL_REGISTER_MOD(ZoomRewriteMod, ZoomRewriteMod::Instance())
