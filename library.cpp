#include "lua_functions.h"
#include "utils.hpp"
#include "lua_script.hpp"

#include <format>
#include <string>
#include <memory>
#include <fstream>
#include <ranges>
#include <sstream>
#include <thread>

#include <windows.h>

static std::unique_ptr<lua_functions> lua = nullptr;
bool IsCheatEnabled = false;
static std::string code = lua_script;
HWND hwnd;

int __cdecl HookedLuaLoadBuffer(int *luaState, const char *buffer, size_t sz, const char *name)
{
    return lua->load_buffer_trampoline(luaState, buffer, sz, name);
}

std::map<std::string, std::pair<double, double>> GetPlayersFromLua(int* lua_state)
{
    if (lua->isnumber(lua_state, -1))
    {
        return {};
    }

    auto result = std::map<std::string, std::pair<double, double>>{};

    lua->pushnil(lua_state);
    while (lua->next(lua_state, -2) != 0)
    {
        auto peerIdRaw = lua->tolstring(lua_state, -2, nullptr);
        auto peerId = std::string(peerIdRaw);

        lua->getfield(lua_state, -1, "x");
        auto x = lua->tonumber(lua_state, -1);
        lua->settop(lua_state, -2);

        lua->getfield(lua_state, -1, "y");
        auto y = lua->tonumber(lua_state, -1);
        lua->settop(lua_state, -2);

        lua->settop(lua_state, -2);

        result[peerId] = std::make_pair(x, y);
    }

    return result;
}

int __cdecl HookedLuaPcall(int *lua_state, int nargs, int nresults, int msgh)
{
    if (!IsCheatEnabled)
    {
        return lua->pcall_trampoline(lua_state, nargs, nresults, msgh);
    }

    // std::ifstream custom(R"(C:\Users\Aleksej\CLionProjects\showdown-hack\custom.lua)");
    // std::stringstream buffer;
    // buffer << custom.rdbuf();
    // code = buffer.str();
    // custom.close();

    auto ret = lua->loadstring(lua_state, code.c_str());
    if (ret)
    {
        auto error = lua->tolstring(lua_state, -1, nullptr);
        lua->settop(lua_state, -2);
        return lua->pcall_trampoline(lua_state, nargs, nresults, msgh);
    }

    ret = lua->pcall_trampoline(lua_state, 0, 1, 0);
    if (ret)
    {
        auto error = lua->tolstring(lua_state, -1, nullptr);
        lua->settop(lua_state, -2);
        return lua->pcall_trampoline(lua_state, nargs, nresults, msgh);
    }

    auto players = GetPlayersFromLua(lua_state);
    lua->settop(lua_state, -2);

    for (auto &[x, y]: players | std::views::values)
    {
        RECT rect;
        GetClientRect(hwnd, &rect);

        if (x < 0 || y < 0 || x > rect.right || y > rect.bottom)
        {
            continue;
        }

        POINT p{static_cast<int>(x), static_cast<int>(y)};
        ClientToScreen(hwnd, &p);

        SetCursorPos(p.x, p.y);
        break;
    }

    return lua->pcall_trampoline(lua_state, nargs, nresults, msgh);
}

bool IsKeyPressed(int key) {
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

[[noreturn]] void CheckHotkey() {
    while (true) {
        if (IsKeyPressed(VK_F5)) {
            IsCheatEnabled = !IsCheatEnabled;
            Sleep(500);
        }
        Sleep(60);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH: {
            auto hExeModule = GetModuleHandle(nullptr);
            hwnd = FindWindow(nullptr, "The Showdown Effect");

            lua = std::make_unique<lua_functions>(lua_functions(
                utils::get_func_from_export<tlua_loadbuffer>("luaL_loadbuffer"),
                utils::get_func_from_export<tlua_pcall>("lua_pcall"),
                utils::get_func_from_export<tlua_loadstring>("luaL_loadstring"),
                utils::get_func_from_export<tlua_settop>("lua_settop"),
                utils::get_func_from_export<tlua_isnumber>("lua_isnumber"),
                utils::get_func_from_export<tlua_tonumber>("lua_tonumber"),
                utils::get_func_from_export<tlua_tolstring>("lua_tolstring"),
                utils::get_func_from_export<tlua_pushnil>("lua_pushnil"),
                utils::get_func_from_export<tlua_next>("lua_next"),
                utils::get_func_from_export<tlua_getfield>("lua_getfield")));

            std::thread(CheckHotkey).detach();

            lua->install_load_buffer_hook(HookedLuaLoadBuffer);
            lua->install_pcall_hook(HookedLuaPcall);
            break;
        }
        case DLL_PROCESS_DETACH:
            break;
        default: break;
    }

    return TRUE;
}
