// Core/ModEntry.cpp

#include "Core/ModContext.hpp"
#include "Core/Config.hpp"
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
        log.info("Core: load() start");

        // 1. Load config & register touch callback
        config::Load();
        touch_controller::Install();

        // Register tick callback
        camera_hook::SetFrameTickCallback([]() {
            zoom_controller::Tick();
            zoom_button::Draw(zoom_controller::IsActive());
        });

        // CATATAN: camera_hook::Install() Sengaja DIPINDAHKAN ke enable()
        // agar tidak membenturkan VTable CameraAPI saat Inbuilt Zoom Mod melakukan nativeInit().

        log.info("Core: load() done");
        return true;
    }

    bool enable() {
        auto& log = core::Log();

        // 2. Registrasi UI Mod Menu
        config::RegisterModMenu();

        // 3. Pasang CameraHook HANYA SETELAH Inbuilt Mods selesai init!
        if (!camera_hook::Install()) {
            log.error("Core: CameraHook::Install() failed during enable()");
            return false;
        }

        return zoom_button::Install();
    }

    bool disable() {
        // Lepas hook saat mod di-disable agar memori tetap bersih
        camera_hook::Uninstall();
        zoom_button::Uninstall();
        return true;
    }

    bool unload() {
        camera_hook::Uninstall();
        return true;
    }
};

PL_REGISTER_MOD(ZoomRewriteMod, ZoomRewriteMod::Instance())
