#include "Core/Config.hpp"
#include "Core/ModContext.hpp"

#include <pl/Config.hpp>
#include <pl/ModMenu.hpp>

namespace config {

Settings g_settings;
static pl::config::ConfigFile<Settings> g_configFile(Settings{}, "config.json");

void Load() {
    g_configFile.load();
    g_settings = g_configFile.get();
}

void Save() {
    g_configFile.get() = g_settings;
    g_configFile.save();
}

void RegisterModMenu() {
    // Pass 2 argumen: ModId dan Display Name untuk UI
    pl::modmenu::ModuleBuilder builder(core::ModId(), "Zoom Settings");

    // 1. Slider Kecepatan Animasi (Int Range 1 - 10)
    builder.addInt("Kecepatan Animasi Zoom", 1, 10, &g_settings.zoomAnimSpeed);

    // 2. Toggle Suara Spyglass (Bool)
    builder.addBool("Efek Suara Spyglass", &g_settings.enableSpyglassSound);

    // 3. Toggle Hide Hand (Bool)
    builder.addBool("Sembunyikan Tangan saat Zoom", &g_settings.hideHandOnZoom);

    // Cast ke void untuk mengabaikan [[nodiscard]] warning
    (void)builder.registerModule();
}

} // namespace config
