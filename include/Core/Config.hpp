#pragma once

#include <optional>
#include <string_view>
#include <pl/Config.hpp>

namespace config {

struct Settings {
    int version = 1;                  // Wajib untuk TypedConfig
    int zoomAnimSpeed = 5;            // Kecepatan animasi (1 - 10)
    bool enableSpyglassSound = true;  // Toggle suara spyglass
    bool hideHandOnZoom = true;       // Toggle sembunyikan tangan
    
    // Gabungkan setting posisi & ukuran tombol ke sini
    float posX = 60.0f;
    float posY = 120.0f;
    float scale = 1.0f;
};

extern Settings g_settings;

void Load();
void Save();
void RegisterModMenu();
void UnregisterModMenu();

} // namespace config

namespace pl::config {

template <> struct Schema<::config::Settings> {
  static constexpr std::string_view title = "Zoom Settings";
  static constexpr std::string_view description = "Pengaturan lengkap untuk Zoom Rewrite";

  static constexpr FieldSchema field(std::string_view name) {
    if (name == "version") {
      return {"Version", "Config schema version", std::nullopt, std::nullopt, true};
    }
    if (name == "zoomAnimSpeed") {
      return {"Zoom Speed", "Kecepatan animasi zoom (1-10)", 1.0, 10.0, false};
    }
    if (name == "enableSpyglassSound") {
      return {"Spyglass Sound", "Efek suara spyglass saat zoom", std::nullopt, std::nullopt, false};
    }
    if (name == "hideHandOnZoom") {
      return {"Hide Hand", "Sembunyikan tangan pemain saat zoom", std::nullopt, std::nullopt, false};
    }
    if (name == "posX") {
      return {"Posisi X", "Posisi X tombol di layar", 0.0, 2500.0, false};
    }
    if (name == "posY") {
      return {"Posisi Y", "Posisi Y tombol di layar", 0.0, 1500.0, false};
    }
    if (name == "scale") {
      return {"Skala Tombol", "Ukuran tombol ZM", 0.5, 3.0, false};
    }
    return {};
  }
};

} // namespace pl::config
