#include "Core/Config.hpp"
#include "Core/ModContext.hpp"

#include <pl/Config.hpp>
#include <pl/ModMenu.hpp>

namespace config {

Settings g_settings;
static pl::config::ConfigFile<Settings> g_configFile;

void Load() {
    g_configFile.load();
    g_settings = g_configFile.value();
}

void Save() {
    g_configFile.value() = g_settings;
    g_configFile.save();
}

void RegisterModMenu() {
    // ConfigFile secara otomatis sudah membuat & membaca config.json di penyimpanan internal.
}

} // namespace config
