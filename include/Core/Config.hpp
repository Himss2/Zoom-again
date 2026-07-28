#pragma once

#include <string>

namespace config {

struct Settings {
    int version = 1;
    int zoomAnimSpeed = 5;
    bool hideHandOnZoom = true;
    
    float posX = 60.0f;
    float posY = 120.0f;
    float scale = 1.0f;
};

extern Settings g_settings;

void Load();
void Save();
void RegisterModMenu();

} // namespace config
