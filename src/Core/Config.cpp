#include "Core/Config.hpp"
#include "Core/ModContext.hpp"

#include <cstdlib>
#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <pl/Config.hpp>
#include <pl/ModMenu.hpp>

namespace config {

Settings g_settings;

static std::unique_ptr<pl::config::ConfigFile<Settings>> g_configFile;

constexpr const char *kModuleId = "zoom_rewrite";

void Load() {
    if (!g_configFile) {
        g_configFile = std::make_unique<pl::config::ConfigFile<Settings>>();
    }
    g_configFile->load();
    g_settings = g_configFile->value();
}

void Save() {
    if (g_configFile) {
        g_configFile->value() = g_settings;
        g_configFile->save();
    }
}

static void onConfigChanged(std::string_view moduleId, std::string_view key, std::string_view value) {
    if (moduleId != kModuleId) return;

    const std::string safeValue(value);
    
    if (key == "zoomAnimSpeed") {
        g_settings.zoomAnimSpeed = std::atoi(safeValue.c_str());
    } else if (key == "hideHandOnZoom") {
        g_settings.hideHandOnZoom = (safeValue == "1" || safeValue == "true");
    } else if (key == "opacity") {
        g_settings.opacity = std::clamp(std::atoi(safeValue.c_str()), 0, 100);
    } else if (key == "pos_x") {
        g_settings.posX = std::strtof(safeValue.c_str(), nullptr);
    } else if (key == "pos_y") {
        g_settings.posY = std::strtof(safeValue.c_str(), nullptr);
    } else if (key == "scale") {
        g_settings.scale = std::strtof(safeValue.c_str(), nullptr);
    }

    Save();
}

void RegisterModMenu() {
    Load();

    std::string strSpeed    = std::to_string(g_settings.zoomAnimSpeed);
    std::string strHideHand = g_settings.hideHandOnZoom ? "true" : "false";
    std::string strOpacity  = std::to_string(g_settings.opacity);
    std::string strX        = std::to_string(g_settings.posX);
    std::string strY        = std::to_string(g_settings.posY);
    std::string strScale    = std::to_string(g_settings.scale);

    (void)pl::modmenu::ModuleBuilder(kModuleId, "Zoom Rewrite")
        .description("Flarial-style smooth zoom & overlay settings")
        .defaultEnabled(true)
        .config("zoomAnimSpeed", "Kecepatan Zoom",
                pl::modmenu::ConfigType::SliderInt, strSpeed, "1", "10")
        .config("hideHandOnZoom", "Sembunyikan Tangan",
                pl::modmenu::ConfigType::Toggle, strHideHand)
        .config("opacity", "Transparansi Tombol (%)",
                pl::modmenu::ConfigType::SliderInt, strOpacity, "0", "100")
        .config("pos_x", "Posisi X Tombol", 
                pl::modmenu::ConfigType::SliderFloat, strX, "0.0", "2500.0")
        .config("pos_y", "Posisi Y Tombol", 
                pl::modmenu::ConfigType::SliderFloat, strY, "0.0", "1500.0")
        .config("scale", "Ukuran Tombol", 
                pl::modmenu::ConfigType::SliderFloat, strScale, "0.5", "3.0")
        .onConfigChanged(onConfigChanged)
        .registerModule();
}

void UnregisterModMenu() {
    pl::modmenu::unregisterModule(kModuleId);
}

} // namespace config
