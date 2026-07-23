#pragma once
// Core/ModContext.hpp
//
// Thin wrapper so every other module (Camera/Input/UI/Zoom) can log and
// query mod info without each pulling in pl/Mod.hpp and repeating
// pl::mod::NativeMod::current() themselves.

#include <pl/Logger.hpp>

namespace core {

// Logger tagged with this mod's own name, safe to call from any module
// after ModEntry's load() has run.
pl::log::Logger& Log();

// This mod's id, as resolved from the manifest (used as ModuleInfo::modId
// / ButtonInfo::modId when registering with ModMenu).
const char* ModId();

} // namespace core
