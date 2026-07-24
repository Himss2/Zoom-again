#include "ZoomButton/ZoomButton.hpp"

#include "Core/ModContext.hpp"

#include <pl/ModMenu.hpp>
#include <array>

namespace zoom_button {
namespace {

constexpr const char* kModuleId = "zoomrewrite.module";

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

void Install() {
    auto& log = core::Log();

    pl::modmenu::ModuleInfo module{};
    module.moduleId = kModuleId;
    module.displayName = "Zoom Rewrite";
    module.description = "Hold + drag (same finger) to zoom, Flarial-style.";
    module.modId = core::ModId();
    module.defaultEnabled = true;

    log.info("ZoomButton: about to registerModule moduleId='{}' displayName='{}' modId='{}'",
             module.moduleId, module.displayName, module.modId);

    pl::modmenu::registerModule(module);

    log.info("ZoomButton: registerModule returned OK");

    // --- DIAGNOSTIC ONLY: testing whether registerModule needs a
    // button registered immediately after it to avoid crashing.
    // OffhandFix (which has never crashed) always calls registerButton
    // right after registerModule in the same function; every crashing
    // test so far (including this project's) called registerModule
    // alone. If this dummy button avoids the crash, that confirms the
    // theory and we'll fold a real button/interaction into ZoomButton
    // properly. If it still crashes, this is ruled out too.
    pl::modmenu::ButtonInfo diagButton{};
    diagButton.buttonId = "zoomrewrite.diag_button";
    diagButton.moduleId = kModuleId;
    diagButton.displayName = "Diagnostic";
    diagButton.modId = core::ModId();
    diagButton.label = "D";
    diagButton.defaultVisible = false; // don't actually show it
    pl::modmenu::registerButton(diagButton);
    log.info("ZoomButton: diagnostic registerButton returned OK");
    // --- end diagnostic ---
}

void Uninstall() {
    pl::modmenu::unregisterButton("zoomrewrite.diag_button");
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
