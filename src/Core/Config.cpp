#include "Core/Config.hpp"
#include "Core/ModContext.hpp"

#include <pl/runtime/ModMenuBridge.hpp>
#include <nlohmann/json.hpp>
#include <fstream>

namespace config {

Settings g_settings;

static const char* kConfigPath = "/sdcard/games/com.mojang/minecraftpe/mods/SmoothZoom/config.json";

void Load() {
    std::ifstream file(kConfigPath);
    if (!file.is_open()) return;

    try {
        nlohmann::json j;
        file >> j;
        
        if (j.contains("zoomAnimSpeed")) g_settings.zoomAnimSpeed = j["zoomAnimSpeed"];
        if (j.contains("hideHandOnZoom")) g_settings.hideHandOnZoom = j["hideHandOnZoom"];
        if (j.contains("posX")) g_settings.posX = j["posX"];
        if (j.contains("posY")) g_settings.posY = j["posY"];
        if (j.contains("scale")) g_settings.scale = j["scale"];
    } catch (...) {
        core::Log().warn("Config: Failed to parse settings file, using defaults");
    }
}

void Save() {
    nlohmann::json j;
    j["zoomAnimSpeed"] = g_settings.zoomAnimSpeed;
    j["hideHandOnZoom"] = g_settings.hideHandOnZoom;
    j["posX"] = g_settings.posX;
    j["posY"] = g_settings.posY;
    j["scale"] = g_settings.scale;

    std::ofstream file(kConfigPath);
    if (file.is_open()) {
        file << j.dump(4);
    }
}

void RegisterModMenu() {
    using namespace pl::modmenu;

    auto builder = ModuleBuilder("SmoothZoom", "Smooth Zoom with Drag Controls");
    
    builder.config("zoomAnimSpeed", "Kecepatan Animasi", ConfigType::Slider, "1,10,1", std::to_string(g_settings.zoomAnimSpeed))
           .config("hideHandOnZoom", "Sembunyikan Tangan", ConfigType::Radio, "0", g_settings.hideHandOnZoom ? "1" : "0")
           .onConfigChanged([](const std::string& key, const std::string& value) {
               if (key == "zoomAnimSpeed") {
                   g_settings.zoomAnimSpeed = std::stoi(value);
               } else if (key == "hideHandOnZoom") {
                   g_settings.hideHandOnZoom = (value == "1");
               }
               Save();
           });

    RegisterModule(builder.build());
}

} // namespace config
