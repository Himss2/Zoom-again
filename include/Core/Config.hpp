#pragma once

namespace config {

struct Settings {
    int version = 1;                  // Wajib untuk TypedConfig concept Preloader SDK
    int zoomAnimSpeed = 5;            // Slider Kecepatan Animasi (1 - 10)
    bool enableSpyglassSound = true;  // Toggle Suara Spyglass
    bool hideHandOnZoom = true;       // Toggle Sembunyikan Tangan saat Zoom
};

extern Settings g_settings;

void Load();
void Save();
void RegisterModMenu();

} // namespace config
