#pragma once

namespace config {

struct Settings {
    int version = 1;                  // Wajib untuk TypedConfig concept Preloader SDK
    int zoomAnimSpeed = 5;            // Kecepatan animasi zoom (1 - 10)
    bool enableSpyglassSound = true;  // Toggle suara spyglass
    bool hideHandOnZoom = true;       // Toggle sembunyikan tangan saat zoom
};

extern Settings g_settings;

void Load();
void Save();
void RegisterModMenu();

} // namespace config
