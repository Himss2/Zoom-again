#include "ZoomButton/ZoomButton.hpp"
#include "Core/Config.hpp"
#include "Core/ModContext.hpp"

#include <pl/ModMenu.hpp>
#include <array>
#include <algorithm>

namespace zoom_button {

namespace {

constexpr const char* kModuleId = "zoom_rewrite";

constexpr float kBaseW = 75.0f;
constexpr float kBaseH = 75.0f;

// Helper untuk menerapkan Alpha (0-100%) ke warna RGB Hex
uint32_t ApplyAlpha(uint32_t rgbColor, int opacityPercent) {
    uint32_t alpha = (static_cast<uint32_t>(opacityPercent) * 255u / 100u) & 0xFFu;
    return (alpha << 24) | (rgbColor & 0x00FFFFFFu);
}

} // namespace

bool Install() {
    return true;
}

void Uninstall() {
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
    float b = 3.0f * s; // Ketebalan Bevel 3D

    int op = config::g_settings.opacity;

    // Skema Warna Tombol Minecraft UI Native (ARGB)
    uint32_t colBlackFrame = ApplyAlpha(0x000000, op);
    
    // Saat Active (Pressed): Bevel terbalik (Atas/Kiri Gelap, Bawah/Kanan Terang)
    uint32_t colHighlight  = ApplyAlpha(isActive ? 0x373737 : 0xFFFFFF, op); // Terang / Gelap
    uint32_t colShadow     = ApplyAlpha(isActive ? 0xFFFFFF : 0x373737, op); // Gelap / Terang
    uint32_t colFill       = ApplyAlpha(isActive ? 0x555555 : 0x8B8B8B, op); // Isian Abu-abu MC
    uint32_t colText       = ApplyAlpha(isActive ? 0xFFFF55 : 0xFFFFFF, op); // Teks Kuning saat ditekan

    std::array<pl::modmenu::DrawCommand, 7> commands{};

    // 1. Bingkai Hitam Luar (Outer Border)
    commands[0].type = pl::modmenu::DrawCommandType::RectFilled;
    commands[0].x = x; commands[0].y = y; commands[0].w = w; commands[0].h = h;
    commands[0].color = colBlackFrame;

    // 2. Isian Utama (Center Fill)
    commands[1].type = pl::modmenu::DrawCommandType::RectFilled;
    commands[1].x = x + b; commands[1].y = y + b;
    commands[1].w = w - (b * 2.0f); commands[1].h = h - (b * 2.0f);
    commands[1].color = colFill;

    // 3. Bevel 3D Atas (Top Edge)
    commands[2].type = pl::modmenu::DrawCommandType::RectFilled;
    commands[2].x = x + b; commands[2].y = y + b;
    commands[2].w = w - (b * 2.0f); commands[2].h = b;
    commands[2].color = colHighlight;

    // 4. Bevel 3D Kiri (Left Edge)
    commands[3].type = pl::modmenu::DrawCommandType::RectFilled;
    commands[3].x = x + b; commands[3].y = y + b;
    commands[3].w = b; commands[3].h = h - (b * 2.0f);
    commands[3].color = colHighlight;

    // 5. Bevel 3D Bawah (Bottom Edge)
    commands[4].type = pl::modmenu::DrawCommandType::RectFilled;
    commands[4].x = x + b; commands[4].y = y + h - (b * 2.0f);
    commands[4].w = w - (b * 2.0f); commands[4].h = b;
    commands[4].color = colShadow;

    // 6. Bevel 3D Kanan (Right Edge)
    commands[5].type = pl::modmenu::DrawCommandType::RectFilled;
    commands[5].x = x + w - (b * 2.0f); commands[5].y = y + b;
    commands[5].w = b; commands[5].h = h - (b * 2.0f);
    commands[5].color = colShadow;

    // 7. Teks "ZM" di Tengah
    commands[6].type = pl::modmenu::DrawCommandType::Text;
    commands[6].x = x + w * 0.5f;
    commands[6].y = y + h * 0.5f;
    commands[6].text = "ZM";
    commands[6].color = colText;
    commands[6].size = 18.0f * s;

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
