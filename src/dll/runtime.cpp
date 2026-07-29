#include "dll/runtime.hpp"

#include <MinHook.h>
#include <windows.h>

#include "overlay/overlay.hpp"

namespace dll
{
    Runtime& Runtime::instance()
    {
        static Runtime runtime;
        return runtime;
    }

    DWORD Runtime::run()
    {
        if (!initialize()) return 0;
        return 0;
    }

    bool Runtime::initialize()
    {
        if (MH_Initialize() != MH_OK) return false;
        if (!lua_hooks_.install(target_assist_))
        {
            MH_Uninitialize();
            return false;
        }
        if (!overlay::install(state_))
        {
            lua_hooks_.uninstall();
            MH_Uninitialize();
            return false;
        }
        return true;
    }
}
