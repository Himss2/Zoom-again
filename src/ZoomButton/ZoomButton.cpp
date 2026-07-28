#include "ZoomButton/ZoomButton.hpp"
#include "Core/Config.hpp"
#include "Core/ModContext.hpp"

#include <pl/ModMenu.hpp>
#include <algorithm>
#include <cmath>
#include <vector>

namespace zoom_button {

namespace {

constexpr const char* kModuleId = "zoom_rewrite";

constexpr float kBaseW = 68.0f;
constexpr float kBaseH = 68.0f;
constexpr float kBaseRadius = 14.0f;          // sudut membulat, ala BedrockTools
constexpr float kBaseOutlineThickness = 2.0f;
constexpr int   kCornerSteps = 8;             // naikkan ke 12-16 kalau masih kelihatan bertangga

// Helper untuk menghasilkan warna ARGB sesuai tingkat transparansi
inline uint32_t MakeColor(uint8_t r, uint8_t g, uint8_t b, float alphaMultiplier, int opacityPercent) {
    float userAlpha = static_cast<float>(opacityPercent) / 100.0f;
    uint32_t a = static_cast<uint32_t>(std::clamp(alphaMultiplier * userAlpha * 255.0f, 0.0f, 255.0f));
    return (a << 24) | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
}

// Menambahkan satu rounded-rect (solid fill) ke buffer command.
// Sudut didekati dengan beberapa strip horizontal tipis dari persamaan lingkaran,
// bukan notch 1-langkah - hasilnya jauh lebih halus.
void AddRoundedRect(std::vector<pl::modmenu::DrawCommand>& out,
                    float x, float y, float w, float h,
                    float radius, uint32_t color) {
    radius = std::min({radius, w * 0.5f, h * 0.5f});
    if (radius < 0.5f) {
        // Terlalu kecil untuk radius berarti, cukup gambar rect biasa
        pl::modmenu::DrawCommand cmd{};
        cmd.type = pl::modmenu::DrawCommandType::RectFilled;
        cmd.x = x; cmd.y = y; cmd.w = w; cmd.h = h;
        cmd.color = color;
        out.push_back(cmd);
        return;
    }

    auto rect = [&](float rx, float ry, float rw, float rh) {
        if (rw <= 0.0f || rh <= 0.0f) return;
        pl::modmenu::DrawCommand cmd{};
        cmd.type = pl::modmenu::DrawCommandType::RectFilled;
        cmd.x = rx; cmd.y = ry; cmd.w = rw; cmd.h = rh;
        cmd.color = color;
        out.push_back(cmd);
    };

    // Badan utama bentuk "plus" (tanpa celah di tengah)
    rect(x + radius, y, w - radius * 2.0f, h);
    rect(x, y + radius, radius, h - radius * 2.0f);
    rect(x + w - radius, y + radius, radius, h - radius * 2.0f);

    // 4 sudut, tiap sudut didekati N strip horizontal tipis
    for (int i = 0; i < kCornerSteps; ++i) {
        float dy = radius * (static_cast<float>(i) / kCornerSteps);
        float dx = std::sqrt(std::max(0.0f, radius * radius - dy * dy));
        float stripH = radius / kCornerSteps + 0.5f; // sedikit overlap, hindari celah subpixel

        rect(x + radius - dx, y + radius - dy - stripH, dx, stripH);              // kiri-atas
        rect(x + w - radius, y + radius - dy - stripH, dx, stripH);               // kanan-atas
        rect(x + radius - dx, y + h - radius + dy, dx, stripH);                   // kiri-bawah
        rect(x + w - radius, y + h - radius + dy, dx, stripH);                    // kanan-bawah
    }
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
    float radius = kBaseRadius * s;
    float outlineThickness = kBaseOutlineThickness * s;

    int userOpacity = config::g_settings.opacity;

    // Warna: outline putih tipis + fill hitam semi-transparan + teks putih
    // (arah styling ala BedrockTools: background/outline/text opacity independen)
    uint32_t colOutline = MakeColor(0xFF, 0xFF, 0xFF, isActive ? 0.55f : 0.30f, userOpacity);
    uint32_t colBg      = MakeColor(0x00, 0x00, 0x00, isActive ? 0.55f : 0.35f, userOpacity);
    uint32_t colText    = MakeColor(0xFF, 0xFF, 0xFF, isActive ? 1.00f : 0.85f, userOpacity);
    uint32_t colShadow  = MakeColor(0x00, 0x00, 0x00, isActive ? 0.60f : 0.40f, userOpacity);

    std::vector<pl::modmenu::DrawCommand> commands;
    commands.reserve(64);

    // Layer outline (rect lebih besar di belakang)
    AddRoundedRect(commands, x, y, w, h, radius, colOutline);
    // Layer fill (inset sebesar outlineThickness)
    AddRoundedRect(commands,
                  x + outlineThickness, y + outlineThickness,
                  w - outlineThickness * 2.0f, h - outlineThickness * 2.0f,
                  radius - outlineThickness, colBg);

    // Teks "ZM" di tengah, dengan drop shadow tipis
    float fontSize = 15.0f * s;
    float textCenterX = x + (w * 0.5f);
    float textCenterY = y + (h * 0.5f);
    float halfTextWidth  = fontSize * 0.55f;
    float halfTextHeight = fontSize * 0.40f;
    float textX = textCenterX - halfTextWidth;
    float textY = textCenterY - halfTextHeight;

    pl::modmenu::DrawCommand shadow{};
    shadow.type = pl::modmenu::DrawCommandType::Text;
    shadow.x = textX + (1.0f * s);
    shadow.y = textY + (1.0f * s);
    shadow.text = "ZM";
    shadow.color = colShadow;
    shadow.size = fontSize;
    commands.push_back(shadow);

    pl::modmenu::DrawCommand text{};
    text.type = pl::modmenu::DrawCommandType::Text;
    text.x = textX;
    text.y = textY;
    text.text = "ZM";
    text.color = colText;
    text.size = fontSize;
    commands.push_back(text);

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
