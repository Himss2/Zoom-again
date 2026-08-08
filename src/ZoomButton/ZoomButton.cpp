#include "ZoomButton/ZoomButton.hpp"
#include "Core/Config.hpp"
#include "Core/ModContext.hpp"

#include <pl/ModMenu.hpp>
#include <array>
#include <algorithm>

namespace zoom_button {

namespace {

constexpr const char* kModuleId = "zoom_rewrite";
constexpr float kBaseDiameter = 68.0f;

inline uint32_t MakeColor(uint8_t r, uint8_t g, uint8_t b, float alphaMultiplier, int opacityPercent) {
    float userAlpha = static_cast<float>(opacityPercent) / 100.0f;
    uint32_t a = static_cast<uint32_t>(std::clamp(alphaMultiplier * userAlpha * 255.0f, 0.0f, 255.0f));
    return (a << 24) | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
}

} // namespace

bool Install() { return true; }
void Uninstall() {}

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
    float diameter = kBaseDiameter * s;
    float radius = diameter * 0.5f;
    float cx = x + radius;
    float cy = y + radius;

    int userOpacity = config::g_settings.opacity;

    // Matches native touch HUD circular button style (jump/sneak/fly):
    // translucent grey fill + slightly lighter outline ring.
    uint32_t colBorder = MakeColor(0x6E, 0x6E, 0x6E, isActive ? 0.65f : 0.40f, userOpacity);
    uint32_t colFill   = MakeColor(0x1C, 0x1C, 0x1C, isActive ? 0.70f : 0.35f, userOpacity);
    uint32_t colText   = MakeColor(0xE0, 0xE0, 0xE0, isActive ? 1.00f : 0.75f, userOpacity);
    uint32_t colShadow = MakeColor(0x3F, 0x3F, 0x3F, isActive ? 0.80f : 0.50f, userOpacity);

    std::array<pl::modmenu::DrawCommand, 4> commands{};

    // Outer ring (drawn full radius; inner fill on top creates the border look).
    commands[0].type = pl::modmenu::DrawCommandType::CircleFilled;
    commands[0].x = cx;
    commands[0].y = cy;
    commands[0].w = radius;
    commands[0].color = colBorder;

    // Inner fill.
    float innerRadius = radius - (1.5f * s);
    commands[1].type = pl::modmenu::DrawCommandType::CircleFilled;
    commands[1].x = cx;
    commands[1].y = cy;
    commands[1].w = innerRadius;
    commands[1].color = colFill;

    float fontSize = 15.0f * s;
    float halfTextWidth  = fontSize * 0.55f;
    float halfTextHeight = fontSize * 0.40f;
    float textX = cx - halfTextWidth;
    float textY = cy - halfTextHeight;

    commands[2].type = pl::modmenu::DrawCommandType::Text;
    commands[2].x = textX + (1.2f * s);
    commands[2].y = textY + (1.2f * s);
    commands[2].text = "ZM";
    commands[2].color = colShadow;
    commands[2].size = fontSize;

    commands[3].type = pl::modmenu::DrawCommandType::Text;
    commands[3].x = textX;
    commands[3].y = textY;
    commands[3].text = "ZM";
    commands[3].color = colText;
    commands[3].size = fontSize;

    pl::modmenu::submitDrawCommands(kModuleId, commands);
}

bool Contains(float px, float py) {
    float x = config::g_settings.posX;
    float y = config::g_settings.posY;
    float s = config::g_settings.scale;
    float radius = (kBaseDiameter * s) * 0.5f;
    float cx = x + radius;
    float cy = y + radius;
    float dx = px - cx;
    float dy = py - cy;
    return (dx * dx + dy * dy) <= (radius * radius); // circular hit-test, matches visual shape now
}

} // namespace zoom_button
