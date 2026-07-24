#include "Core/ModContext.hpp"

#include <pl/Mod.hpp>
#include <string>

namespace core {
namespace {

pl::mod::NativeMod* g_mod = nullptr;

} // namespace

void Init() {
    g_mod = pl::mod::NativeMod::current();
}

pl::log::Logger& Log() {
    return g_mod->getLogger();
}

const char* ModId() {
    static const std::string id = g_mod->getId();
    return id.c_str();
}

} // namespace core
