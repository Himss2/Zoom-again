#include "ZoomButton/ZoomButton.hpp"
#include "Core/Config.hpp"
#include "Core/ModContext.hpp"

#include <pl/ModMenu.hpp>
#include <array>
#include <algorithm>

namespace zoom_button {

namespace {

constexpr const char* kModuleId = "zoom_rewrite";

constexpr float kBaseW = 68.0f;
constexpr float kBaseH = 68.0f;

// Helper untuk menghasilkan warna ARGB sesuai tingkat transparansi
inline uint32_t MakeColor(uint8_t r, uint8_t g, uint8_t b, float alphaMultiplier, int opacityPercent) {
    float userAlpha = static_cast<float>(opacityPercent) / 100.0f;
    uint32_t a = static_cast<uint32_t>(std::clamp(alphaMultiplier * userAlpha * 255.0f, 0.0f, 255.0f));
    return (a << 24) | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
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

    int userOpacity = config::g_settings.opacity;

    // Ukuran Potongan Piksel Sudut (Pixel-Art Notch Step)
    // Disesuaikan dengan skala agar tetap tampak proporsional seperti piksel Minecraft
    float pStep  = std::max(2.0f, 2.5f * s); // Ukuran notch luar
    float b      = 1.5f * s;                 // Ketebalan border
    float ipStep = std::max(1.0f, 1.5f * s); // Ukuran notch dalam

    // Warna Touch HUD Native Minecraft
    uint32_t colBorder = MakeColor(0x6E, 0x6E, 0x6E, isActive ? 0.65f : 0.40f, userOpacity); // Garis Tepi
    uint32_t colFill   = MakeColor(0x1C, 0x1C, 0x1C, isActive ? 0.70f : 0.35f, userOpacity); // Latar Belakang
    uint32_t colText   = MakeColor(0xE0, 0xE0, 0xE0, isActive ? 1.00f : 0.75f, userOpacity); // Teks Utama
    uint32_t colShadow = MakeColor(0x3F, 0x3F, 0x3F, isActive ? 0.80f : 0.50f, userOpacity); // Bayangan Teks MC

    std::array<pl::modmenu::DrawCommand, 8> commands{};

    // =========================================================================
    // 1. PIXELATED OUTER BORDER (Dipotong Tangga Piksel di 4 Sudut)
    // =========================================================================
    // Strip Atas
    commands[0].type = pl::modmenu::DrawCommandType::RectFilled;
    commands[0].x = x + pStep; commands[0].y = y;
    commands[0].w = w - (pStep * 2.0f); commands[0].h = pStep;
    commands[0].color = colBorder;

    // Strip Tengah Utama
    commands[1].type = pl::modmenu::DrawCommandType::RectFilled;
    commands[1].x = x; commands[1].y = y + pStep;
    commands[1].w = w; commands[1].h = h - (pStep * 2.0f);
    commands[1].color = colBorder;

    // Strip Bawah
    commands[2].type = pl::modmenu::DrawCommandType::RectFilled;
    commands[2].x = x + pStep; commands[2].y = y + h - pStep;
    commands[2].w = w - (pStep * 2.0f); commands[2].h = pStep;
    commands[2].color = colBorder;

    // =========================================================================
    // 2. PIXELATED INNER FILL (Isian Dalam Bertangga Piksel)
    // =========================================================================
    float ix = x + b;
    float iy = y + b;
    float iw = w - (b * 2.0f);
    float ih = h - (b * 2.0f);

    // Strip Dalam Atas
    commands[3].type = pl::modmenu::DrawCommandType::RectFilled;
    commands[3].x = ix + ipStep; commands[3].y = iy;
    commands[3].w = iw - (ipStep * 2.0f); commands[3].h = ipStep;
    commands[3].color = colFill;

    // Strip Dalam Tengah
    commands[4].type = pl::modmenu::DrawCommandType::RectFilled;
    commands[4].x = ix; commands[4].y = iy + ipStep;
    commands[4].w = iw; commands[4].h = ih - (ipStep * 2.0f);
    commands[4].color = colFill;

    // Strip Dalam Bawah
    commands[5].type = pl::modmenu::DrawCommandType::RectFilled;
    commands[5].x = ix + ipStep; commands[5].y = iy + ih - ipStep;
    commands[5].w = iw - (ipStep * 2.0f); commands[5].h = ipStep;
    commands[5].color = colFill;

    // =========================================================================
    // 3. TEKS "ZM" TEPAT DI TENGAH DENGAN DROP SHADOW
    // =========================================================================
    float fontSize = 15.0f * s;
    float textCenterX = x + (w * 0.5f);
    float textCenterY = y + (h * 0.5f);

    float halfTextWidth  = fontSize * 0.55f;
    float halfTextHeight = fontSize * 0.40f;

    float textX = textCenterX - halfTextWidth;
    float textY = textCenterY - halfTextHeight;

    // Bayangan Teks MC (Offset +1px)
    commands[6].type = pl::modmenu::DrawCommandType::Text;
    commands[6].x = textX + (1.2f * s);
    commands[6].y = textY + (1.2f * s);
    commands[6].text = "ZM";
    commands[6].color = colShadow;
    commands[6].size = fontSize;

    // Teks Utama "ZM"
    commands[7].type = pl::modmenu::DrawCommandType::Text;
    commands[7].x = textX;
    commands[7].y = textY;
    commands[7].text = "ZM";
    commands[7].color = colText;
    commands[7].size = fontSize;

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
