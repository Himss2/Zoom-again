#include "ZoomButton/ZoomButton.hpp"

#include "Core/ModContext.hpp"

#include <pl/Config.hpp>
#include <pl/ModMenu.hpp>
#include <array>
#include <optional>

namespace zoom_button {
namespace {

constexpr const char* kModuleId = "zoomrewrite.hud";

constexpr float kZoneX = 60.0f;
constexpr float kZoneY = 120.0f;
constexpr float kZoneW = 150.0f;
constexpr float kZoneH = 90.0f;

constexpr uint32_t kColorIdle   = 0x88666666u;
constexpr uint32_t kColorActive = 0x8800AA00u;
constexpr uint32_t kColorText   = 0xFFFFFFFFu;

struct ZoomButtonConfig {
    int version = 1;
    bool showOverlay = true;
};

std::optional<pl::config::ConfigFile<ZoomButtonConfig>> g_config;

} // namespace

bool Install() {
    auto& log = core::Log();

    g_config.emplace();
    if (!g_config->load()) {
        log.error("ZoomButton: config load failed");
        return false;
    }
    log.info("ZoomButton: config loaded (showOverlay={})", g_config->value().showOverlay);

    bool ok = pl::modmenu::ModuleBuilder(kModuleId, "Zoom Rewrite")
        .modId(core::ModId())
        .description("Hold + drag (same finger) to zoom, Flarial-style.")
        .defaultEnabled(g_config->value().showOverlay)
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

void Draw(bool isActive) {
    std::array<pl::modmenu::DrawCommand, 2> commands{};

    commands[0].type = pl::modmenu::DrawCommandType::RectFilled;
    commands[0].x = kZoneX;
    commands[0].y = kZoneY;
    commands[0].w = kZoneW;
    commands[0].h = kZoneH;
    commands[0].color = isActive ? kColorActive : kColorIdle;

    commands[1].type = pl::modmenu::DrawCommandType::Text;
    commands[1].x = kZoneX + kZoneW * 0.5f;
    commands[1].y = kZoneY + kZoneH * 0.5f;
    commands[1].text = "ZM";
    commands[1].color = kColorText;
    commands[1].size = 24.0f;

    pl::modmenu::submitDrawCommands(kModuleId, commands);
}

bool Contains(float x, float y) {
    return x >= kZoneX && x <= (kZoneX + kZoneW) &&
           y >= kZoneY && y <= (kZoneY + kZoneH);
}

} // namespace zoom_button
