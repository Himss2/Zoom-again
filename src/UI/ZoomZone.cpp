#include "UI/ZoomZone.hpp"

#include "Core/ModContext.hpp"
#include "Zoom/ZoomState.hpp"

#include <pl/ModMenu.hpp>
#include <array>

namespace ui {
namespace {

constexpr const char* kModuleId = "smoothzoom.module";

// Screen-space rectangle for the hand-drawn zone. Adjust to taste -
// not adjustable via the launcher's HUD editor since it's not a real
// ButtonInfo widget.
constexpr float kZoneX = 60.0f;
constexpr float kZoneY = 120.0f;
constexpr float kZoneW = 150.0f;
constexpr float kZoneH = 90.0f;

constexpr uint32_t kColorIdle   = 0x88666666u; // ARGB, semi-transparent grey
constexpr uint32_t kColorActive = 0x8800AA00u; // ARGB, semi-transparent green
constexpr uint32_t kColorText   = 0xFFFFFFFFu;

} // namespace

void RegisterZoomModule() {
    auto& log = core::Log();

    pl::modmenu::ModuleInfo module{};
    module.moduleId = kModuleId;
    module.displayName = "Smooth Zoom";
    module.description = "Hold + drag (same finger) to zoom, Flarial-style.";
    module.modId = core::ModId();
    module.defaultEnabled = true;

    log.info("UI: about to registerModule moduleId='{}' displayName='{}' modId='{}'",
              module.moduleId, module.displayName, module.modId);

    pl::modmenu::registerModule(module);

    log.info("UI: registerModule returned OK");
}

void UnregisterZoomModule() {
    pl::modmenu::unregisterModule(kModuleId);
}

void DrawZoomZone() {
    bool isActive = zoom::IsActive();

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

bool PointInZoomZone(float x, float y) {
    return x >= kZoneX && x <= (kZoneX + kZoneW) &&
           y >= kZoneY && y <= (kZoneY + kZoneH);
}

} // namespace ui
