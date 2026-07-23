#include "Core/ModContext.hpp"

#include <pl/Mod.hpp>
#include <string>

namespace core {

pl::log::Logger& Log() {
    return pl::mod::NativeMod::current()->getLogger();
}

const char* ModId() {
    static const std::string id = pl::mod::NativeMod::current()->getId();
    return id.c_str();
}

} // namespace core
