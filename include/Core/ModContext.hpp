#pragma once
// Core/ModContext.hpp
//
// Thin wrapper so every other module can log and query mod info without
// each pulling in pl/Mod.hpp and repeating pl::mod::NativeMod::current()
// themselves.
//
// IMPORTANT: pl::mod::NativeMod::current() is only valid to call DURING
// a lifecycle call (load()/enable()/disable()/unload()) - it returned
// null (crashing on the next dereference) when called later from an
// async callback (confirmed on-device: crashed inside Log() when called
// from TouchController's touch callback, which runs on its own
// dispatch path outside the lifecycle call stack). Init() must be
// called once, at the very start of load(), while current() is still
// valid - every other module's use of Log()/ModId() after that is safe
// because it reads the cached pointer instead of calling current() again.

#include <pl/Logger.hpp>

namespace core {

// Caches the current NativeMod pointer. Call exactly once, as the very
// first thing in load() - before any other module's Install()/etc,
// since they may call Log() themselves.
void Init();

// Safe to call any time after Init() has run, from any thread/callback.
pl::log::Logger& Log();
const char* ModId();

} // namespace core
