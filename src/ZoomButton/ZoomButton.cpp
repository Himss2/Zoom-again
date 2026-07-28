#include "ZoomButton/ZoomButton.hpp"
#include "Core/Config.hpp"
#include "Core/ModContext.hpp"

#include <pl/ModMenu.hpp>
#include <array>
#include <algorithm>

namespace zoom_button {

namespace {

constexpr const char* kModuleId = "zoom_rewrite";

constexpr float kBaseW = 80.0f;
constexpr float kBaseH = 80.0f;

constexpr uint32_t kColorBorder   = 0x66FFFFFFu;
constexpr uint32_t kColorBgIdle   = 0x33000000u;
constexpr uint32_t kColorBgActive = 0x7700AA00u;
constexpr uint32_t kColorText     = 0xEEFFFFFFu;

} // namespace

bool Install() {
    // Pendaftaran Mod Menu dikendalikan sepenuhnya oleh Config.cpp
    return true;
}

void Uninstall() {
    // Bersihkan jika diperlukan
}

float GetX() { return config::g_settings.posX; }
float GetY() { return config::g_settings.posY; }
float GetScale() { return config::g_settings.scale; }

void SetPosition(float x, float y) {
    config::g_settings.posX = x;
    config::g_settings.posY = y;
    config::Save();
}

void SetScale(float scale) {
    config::g_settings.scale = std::clamp(scale, 0.5f, 3.0f);
    config::Save();
}

void Draw(bool isActive) {
    float x = config::g_settings.posX;
    float y = config::g_settings.posY;
    float s = config::g_settings.scale;
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
    float x = config::g_settings.posX;
    float y = config::g_settings.posY;
    float s = config::g_settings.scale;
    float w = kBaseW * s;
    float h = kBaseH * s;

    return px >= x && px <= (x + w) &&
           py >= y && py <= (y + h);
}

} // namespace zoom_button
