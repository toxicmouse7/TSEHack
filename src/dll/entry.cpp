#include <windows.h>

#include "dll/runtime.hpp"

namespace
{
    DWORD WINAPI runtime_thread(LPVOID) { return dll::Runtime::instance().run(); }
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(module);
        const auto thread = CreateThread(nullptr, 0, runtime_thread, nullptr, 0, nullptr);
        if (!thread) return FALSE;
        CloseHandle(thread);
    }
    return TRUE;
}
