#include "Core/Config.hpp"
#include "Core/ModContext.hpp"

#include <cstdlib>
#include <string>
#include <string_view>
#include <pl/Config.hpp>
#include <pl/ModMenu.hpp>

namespace config {

Settings g_settings;
static pl::config::ConfigFile<Settings> g_configFile;

constexpr const char *kModuleId = "zoom_rewrite.settings";

void Load() {
    g_configFile.load();
    g_settings = g_configFile.value();
}

void Save() {
    g_configFile.value() = g_settings;
    g_configFile.save();
}

static void onConfigChanged(std::string_view moduleId, std::string_view key, std::string_view value) {
    if (moduleId != kModuleId) return;

    const std::string safeValue(value);
    
    if (key == "zoomAnimSpeed") {
        g_settings.zoomAnimSpeed = std::atoi(safeValue.c_str());
    } else if (key == "enableSpyglassSound") {
        g_settings.enableSpyglassSound = (safeValue == "1" || safeValue == "true");
    } else if (key == "hideHandOnZoom") {
        g_settings.hideHandOnZoom = (safeValue == "1" || safeValue == "true");
    }

    Save();
}

void RegisterModMenu() {
    pl::modmenu::ModuleBuilder(kModuleId, "Zoom Settings")
        .description("Atur kecepatan zoom dan opsi tampilan")
        .defaultEnabled(true)
        .config("zoomAnimSpeed", "Kecepatan Zoom",
                pl::modmenu::ConfigType::SliderInt, "5", "1", "10")
        .config("enableSpyglassSound", "Suara Spyglass",
                pl::modmenu::ConfigType::Radio, "1", "Matikan,Aktif")
        .config("hideHandOnZoom", "Sembunyikan Tangan",
                pl::modmenu::ConfigType::Radio, "1", "Matikan,Aktif")
        .onConfigChanged(onConfigChanged)
        .registerModule();
}

void UnregisterModMenu() {
    pl::modmenu::unregisterModule(kModuleId);
}

} // namespace config
