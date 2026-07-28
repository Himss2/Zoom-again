#pragma once

namespace config {

struct Settings {
    int zoomAnimSpeed = 5;           // Range 1 (Lambat) - 10 (Cepat)
    bool enableSpyglassSound = true;  // Toggle Suara Spyglass
    bool hideHandOnZoom = true;       // Toggle Sembunyikan Tangan saat Zoom
};

extern Settings g_settings;

void Load();
void Save();
void RegisterModMenu();

} // namespace config
