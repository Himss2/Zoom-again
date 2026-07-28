#include "Core/Config.hpp"
#include "Core/ModContext.hpp"

#include <pl/Config.hpp>
#include <pl/ModMenu.hpp>

namespace config {

Settings g_settings;
static pl::config::ConfigFile g_configFile("config.json");

void Load() {
    g_configFile.load();
    g_settings.zoomAnimSpeed = g_configFile.get<int>("zoomAnimSpeed", 5);
    g_settings.enableSpyglassSound = g_configFile.get<bool>("enableSpyglassSound", true);
    g_settings.hideHandOnZoom = g_configFile.get<bool>("hideHandOnZoom", true);
}

void Save() {
    g_configFile.set("zoomAnimSpeed", g_settings.zoomAnimSpeed);
    g_configFile.set("enableSpyglassSound", g_settings.enableSpyglassSound);
    g_configFile.set("hideHandOnZoom", g_settings.hideHandOnZoom);
    g_configFile.save();
}

void RegisterModMenu() {
    // Daftarkan Pengaturan ke UI Mod Menu Preloader
    pl::modmenu::ModuleBuilder builder(core::ModId());
    
    // 1. Slider Kecepatan Animasi (1 - 10)
    builder.addSlider("Kecepatan Animasi Zoom", 1, 10, &g_settings.zoomAnimSpeed, [](int val) {
        g_settings.zoomAnimSpeed = val;
        Save();
    });

    // 2. Toggle Suara Spyglass
    builder.addToggle("Efek Suara Spyglass", &g_settings.enableSpyglassSound, [](bool val) {
        g_settings.enableSpyglassSound = val;
        Save();
    });

    // 3. Toggle Hide Hand
    builder.addToggle("Sembunyikan Tangan saat Zoom", &g_settings.hideHandOnZoom, [](bool val) {
        g_settings.hideHandOnZoom = val;
        Save();
    });

    builder.registerModule();
}

} // namespace config
