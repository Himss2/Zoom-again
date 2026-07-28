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
    pl::modmenu::ModuleBuilder builder(core::ModId(), "Zoom Settings");

    // Preloader SDK merefleksikan struct Settings secara otomatis
    builder.addConfig(g_configFile);

    (void)builder.registerModule();
}

} // namespace config
