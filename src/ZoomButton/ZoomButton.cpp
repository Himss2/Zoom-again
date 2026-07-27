#include "ZoomButton/ZoomButton.hpp"

#include "Core/ModContext.hpp"

#include <pl/ModMenu.hpp>
#include <array>

// NOTE: pl::config::ConfigFile<T> usage was tried here (per the
// official checklist: "Load config before registering runtime UI")
// but pl::Config.hpp's boost::pfr-based reflection failed to compile
// against this project's boost_pfr 2.2.0 + NDK r27c combination - a
// template instantiation error inside boost/pfr/detail itself, the
// first time ANY module in this whole debugging history actually
// instantiated ConfigFile<SomeStruct> (earlier projects fetched
// boost_pfr only because pl/Config.hpp includes it unconditionally,
// but never triggered this specific code path). Dropped for now to
// isolate whether ModuleBuilder + enable()-timing alone (the other two
// factors) already avoid the Blocker #1 crash; config integration to
// be revisited separately if so.

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

} // namespace

bool Install() {
    auto& log = core::Log();

    bool ok = pl::modmenu::ModuleBuilder(kModuleId, "Zoom Rewrite")
        .modId(core::ModId())
        .description("Hold + drag (same finger) to zoom, Flarial-style.")
        .defaultEnabled(true)
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
