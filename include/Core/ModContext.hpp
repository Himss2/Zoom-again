#pragma once
// Core/ModContext.hpp
//
// Thin wrapper so every other module can log and query mod info without
// each pulling in pl/Mod.hpp and repeating pl::mod::NativeMod::current()
// themselves.

#include <pl/Logger.hpp>

namespace core {

pl::log::Logger& Log();
const char* ModId();

} // namespace core
