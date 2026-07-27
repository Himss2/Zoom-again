#include "ZoomButton/ZoomButton.hpp"

#include "Core/ModContext.hpp"

#include <pl/Config.hpp>
#include <pl/ModMenu.hpp>
#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <algorithm>
#include <cstdlib>

namespace zoom_button {

struct ZoomButtonConfig {
    int version = 1;
    bool showOverlay = true;
    float x = 60.0f;
    float y = 120.0f;
    float scale = 1.0f;
};

namespace {

constexpr const char* kModuleId = "zoomrewrite.hud";

constexpr float kBaseW = 80.0f;
constexpr float kBaseH = 80.0f;

constexpr uint32_t kColorBorder   = 0x66FFFFFFu;
constexpr uint32_t kColorBgIdle   = 0x33000000u;
constexpr uint32_t kColorBgActive = 0x7700AA00u;
constexpr uint32_t kColorText     = 0xEEFFFFFFu;

std::optional<pl::config::ConfigFile<ZoomButtonConfig>> g_config;

// CALLBACK SLIDER (HANYA UPDATE VARIABLE IN-MEMORY, TANPA DISK I/O)
void onConfigChanged(std::string_view moduleId, std::string_view key, std::string_view value) {
    if (moduleId != kModuleId || !g_config) return;

    const std::string safeValue(value);
    if (key == "pos_x") {
        g_config->value().x = std::strtof(safeValue.c_str(), nullptr);
    } else if (key == "pos_y") {
        g_config->value().y = std::strtof(safeValue.c_str(), nullptr);
    } else if (key == "scale") {
        g_config->value().scale = std::strtof(safeValue.c_str(), nullptr);
    }
    // Dihapus g_config->save() dari sini agar game tidak patah-patah!
}

} // namespace

bool Install() {
    auto& log = core::Log();

    g_config.emplace();
    if (!g_config->load()) {
        log.error("ZoomButton: config load failed");
        return false;
    }

    std::string modIdStr = core::ModId();
    if (modIdStr.empty()) {
        modIdStr = "zoom_rewrite";
    }

    std::string strX = std::to_string(g_config->value().x);
    std::string strY = std::to_string(g_config->value().y);
    std::string strScale = std::to_string(g_config->value().scale);

    bool ok = pl::modmenu::ModuleBuilder(kModuleId, "Zoom Rewrite")
        .modId(modIdStr)
        .description("Hold + drag to zoom in game.")
        .defaultEnabled(g_config->value().showOverlay)
        .config("pos_x", "Position X", pl::modmenu::ConfigType::SliderFloat, strX, "0.0", "2500.0")
        .config("pos_y", "Position Y", pl::modmenu::ConfigType::SliderFloat, strY, "0.0", "1500.0")
        .config("scale", "Button Scale", pl::modmenu::ConfigType::SliderFloat, strScale, "0.5", "3.0")
        .onConfigChanged(onConfigChanged)
        .registerModule();

    if (!ok) {
        log.error("ZoomButton: ModuleBuilder::registerModule() returned false");
        return false;
    }

    return true;
}

void Uninstall() {
    // Simpan ke disk hanya saat mod di-uninstall/ditutup
    if (g_config) {
        g_config->save();
    }
    pl::modmenu::unregisterModule(kModuleId);
}

float GetX() { return g_config ? g_config->value().x : 60.0f; }
float GetY() { return g_config ? g_config->value().y : 120.0f; }
float GetScale() { return g_config ? g_config->value().scale : 1.0f; }

void SetPosition(float x, float y) {
    if (g_config) {
        g_config->value().x = x;
        g_config->value().y = y;
    }
}

void SetScale(float scale) {
    if (g_config) {
        g_config->value().scale = std::clamp(scale, 0.5f, 3.0f);
    }
}

void Draw(bool isActive) {
    if (!g_config) return;

    float x = g_config->value().x;
    float y = g_config->value().y;
    float s = g_config->value().scale;
    float w = kBaseW * s;
    float h = kBaseH * s;
    float borderWidth = 2.0f * s;

    std::array<pl::modmenu::DrawCommand, 3> commands{};

    commands[0].type = pl::modmenu::DrawCommandType::RectFilled;
    commands[0].x = x;
    commands[0].y = y;
    commands[0].w = w;
    commands[0].h = h;
    commands[0].color = kColorBorder;

    commands[1].type = pl::modmenu::DrawCommandType::RectFilled;
    commands[1].x = x + borderWidth;
    commands[1].y = y + borderWidth;
    commands[1].w = w - (borderWidth * 2.0f);
    commands[1].h = h - (borderWidth * 2.0f);
    commands[1].color = isActive ? kColorBgActive : kColorBgIdle;

    commands[2].type = pl::modmenu::DrawCommandType::Text;
    commands[2].x = x + w * 0.5f;
    commands[2].y = y + h * 0.5f;
    commands[2].text = "ZM";
    commands[2].color = kColorText;
    commands[2].size = 18.0f * s;

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
