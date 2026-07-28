#include "Core/Config.hpp"
#include "Core/ModContext.hpp"

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
        
        core::Log().info("Config: Loaded successfully from file");
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
    // Dipanggil saat mod initialize untuk membaca config
    Load();
}

} // namespace config
