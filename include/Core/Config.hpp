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
};

extern Settings g_settings;

void Load();
void Save();
void RegisterModMenu();
void UnregisterModMenu();

} // namespace config

// Schema metadata untuk Preloader Config Generator
namespace pl::config {

template <> struct Schema<::config::Settings> {
  static constexpr std::string_view title = "Zoom Settings";
  static constexpr std::string_view description = "Pengaturan untuk mod Smooth Zoom";

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
    return {};
  }
};

} // namespace pl::config
