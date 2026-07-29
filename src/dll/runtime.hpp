#pragma once

#include <windows.h>

#include "core/cheat_state.hpp"
#include "features/target_assist.hpp"
#include "hooks/lua_hooks.hpp"

namespace dll
{
    class Runtime
    {
    public:
        static Runtime& instance();
        DWORD run();

    private:
        bool initialize();

        core::CheatState state_;
        features::TargetAssist target_assist_{state_};
        hooks::LuaHooks lua_hooks_{state_};
    };
}
