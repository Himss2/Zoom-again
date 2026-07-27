#include "ZoomButton/ZoomButton.hpp"

#include "Core/ModContext.hpp"

#include <pl/Config.hpp>
#include <pl/ModMenu.hpp>
#include <array>
#include <optional>
#include <string>
#include <algorithm>

namespace zoom_button {

struct ZoomButtonConfig {
    int version = 1;
    bool showOverlay = true;
    float x = 60.0f;
    float y = 120.0f;
    float scale = 1.0f; // Default scale (100%)
};

namespace {

constexpr const char* kModuleId = "zoomrewrite.hud";

// Ukuran dasar tombol sebelum di-scale
constexpr float kBaseW = 150.0f;
constexpr float kBaseH = 90.0f;

constexpr uint32_t kColorIdle   = 0x88666666u;
constexpr uint32_t kColorActive = 0x8800AA00u;
constexpr uint32_t kColorText   = 0xFFFFFFFFu;

std::optional<pl::config::ConfigFile<ZoomButtonConfig>> g_config;

} // namespace

bool Install() {
    auto& log = core::Log();

    g_config.emplace();
    if (!g_config->load()) {
        log.error("ZoomButton: config load failed");
        return false;
    }
    log.info("ZoomButton: config loaded (x={}, y={}, scale={})", 
              g_config->value().x, g_config->value().y, g_config->value().scale);

    std::string modIdStr = core::ModId();
    if (modIdStr.empty()) {
        modIdStr = "zoom_rewrite";
    }

    // =========================================================================
    // MOD MENU MODULE BUILDER WITH SLIDERS
    // =========================================================================
    bool ok = pl::modmenu::ModuleBuilder(kModuleId, "Zoom Rewrite")
        .modId(modIdStr)
        .description("Hold + drag (same finger) to zoom, Flarial-style.")
        .defaultEnabled(g_config->value().showOverlay)
        // Slider Pengaturan Posisi & Ukuran
        .addSlider("Position X", &g_config->value().x, 0.0f, 2500.0f, 10.0f)
        .addSlider("Position Y", &g_config->value().y, 0.0f, 1500.0f, 10.0f)
        .addSlider("Button Scale", &g_config->value().scale, 0.5f, 3.0f, 0.1f)
        .onSave([]() {
            if (g_config) g_config->save();
        })
        .registerModule();

    if (!ok) {
        log.error("ZoomButton: ModuleBuilder::registerModule() returned false");
        return false;
    }

    log.info("ZoomButton: registerModule (via ModuleBuilder) returned OK");
    return true;
}

void Uninstall() {
    pl::modmenu::unregisterModule(kModuleId);
}

float GetX() { return g_config ? g_config->value().x : 60.0f; }
float GetY() { return g_config ? g_config->value().y : 120.0f; }
float GetScale() { return g_config ? g_config->value().scale : 1.0f; }

void SetPosition(float x, float y) {
    if (g_config) {
        g_config->value().x = x;
        g_config->value().y = y;
        g_config->save();
    }
}

void SetScale(float scale) {
    if (g_config) {
        g_config->value().scale = std::clamp(scale, 0.5f, 3.0f);
        g_config->save();
    }
}

void Draw(bool isActive) {
    if (!g_config) return;

    float x = g_config->value().x;
    float y = g_config->value().y;
    float s = g_config->value().scale;
    float w = kBaseW * s;
    float h = kBaseH * s;

    std::array<pl::modmenu::DrawCommand, 2> commands{};

    commands[0].type = pl::modmenu::DrawCommandType::RectFilled;
    commands[0].x = x;
    commands[0].y = y;
    commands[0].w = w;
    commands[0].h = h;
    commands[0].color = isActive ? kColorActive : kColorIdle;

    commands[1].type = pl::modmenu::DrawCommandType::Text;
    commands[1].x = x + w * 0.5f;
    commands[1].y = y + h * 0.5f;
    commands[1].text = "ZM";
    commands[1].color = kColorText;
    commands[1].size = 24.0f * s;

    pl::modmenu::submitDrawCommands(kModuleId, commands);
}

bool Contains(float px, float py) {
    if (!g_config) return false;

    float x = g_config->value().x;
    float y = g_config->value().y;
    float s = g_config->value().scale;
    float w = kBaseW * s;
    float h = kBaseH * s;

    return px >= x && px <= (x + w) &&
           py >= y && py <= (y + h);
}

} // namespace zoom_button
