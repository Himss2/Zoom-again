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
    float scale = 1.0f;
};

namespace {

constexpr const char* kModuleId = "zoomrewrite.hud";

// Ukuran dasar tombol
constexpr float kBaseW = 100.0f;
constexpr float kBaseH = 100.0f; // Dibuat agak membulat/presisi persegi seperti tombol pedang/crosshair

// WARNA GAYA MINECRAFT HUD (Hex ARGB)
constexpr uint32_t kColorBgIdle     = 0x44000000u; // Hitam Transparan (Mirip Tombol MC)
constexpr uint32_t kColorBgActive   = 0x77008800u; // Hijau Transparan saat ditekan
constexpr uint32_t kColorBorder     = 0x55FFFFFFu; // Garis pinggir putih halus
constexpr uint32_t kColorText       = 0xEEFFFFFFu; // Teks Putih Terang

std::optional<pl::config::ConfigFile<ZoomButtonConfig>> g_config;

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

    bool ok = pl::modmenu::ModuleBuilder(kModuleId, "Zoom Rewrite")
        .modId(modIdStr)
        .description("Hold + drag to zoom in game.")
        .defaultEnabled(g_config->value().showOverlay)
        .registerModule();

    if (!ok) {
        log.error("ZoomButton: ModuleBuilder::registerModule() returned false");
        return false;
    }

    log.info("ZoomButton: registered module successfully");
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

    // Menyiapkan 3 elemen gambar: Background, Border, dan Teks
    std::array<pl::modmenu::DrawCommand, 3> commands{};

    // 1. Background Kotak Utama (Transparan)
    commands[0].type = pl::modmenu::DrawCommandType::RectFilled;
    commands[0].x = x;
    commands[0].y = y;
    commands[0].w = w;
    commands[0].h = h;
    commands[0].color = isActive ? kColorBgActive : kColorBgIdle;

    // 2. Garis Pinggir / Border
    commands[1].type = pl::modmenu::DrawCommandType::Rect;
    commands[1].x = x;
    commands[1].y = y;
    commands[1].w = w;
    commands[1].h = h;
    commands[1].color = kColorBorder;

    // 3. Teks "ZM" di Tengah
    commands[2].type = pl::modmenu::DrawCommandType::Text;
    commands[2].x = x + w * 0.5f;
    commands[2].y = y + h * 0.5f;
    commands[2].text = "ZM";
    commands[2].color = kColorText;
    commands[2].size = 20.0f * s;

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
