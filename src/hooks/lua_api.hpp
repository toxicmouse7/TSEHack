#pragma once

#include <cstddef>

namespace hooks
{
    using LuaLoadBuffer = int(__cdecl*)(int* lua_state, const char* buffer, size_t size, const char* name);
    using LuaPCall = int(__cdecl*)(int* lua_state, int nargs, int nresults, int msgh);
    using LuaLRef = int(__cdecl*)(int* lua_state, int index);
    using LuaRawGetI = void(__cdecl*)(int* lua_state, int index, int reference);
    using LuaSetTop = void(__cdecl*)(int* lua_state, int index);
    using LuaIsNumber = int(__cdecl*)(int* lua_state, int index);
    using LuaToNumber = double(__cdecl*)(int* lua_state, int index);
    using LuaToString = const char*(__cdecl*)(int* lua_state, int index, size_t* length);
    using LuaPushNil = void(__cdecl*)(int* lua_state);
    using LuaNext = int(__cdecl*)(int* lua_state, int index);
    using LuaGetField = int(__cdecl*)(int* lua_state, int index, const char* key);

    struct LuaApi
    {
        LuaLoadBuffer load_buffer{};
        LuaPCall pcall{};
        LuaLRef ref{};
        LuaRawGetI raw_get_i{};
        LuaSetTop set_top{};
        LuaIsNumber is_number{};
        LuaToNumber to_number{};
        LuaToString to_string{};
        LuaPushNil push_nil{};
        LuaNext next{};
        LuaGetField get_field{};

        static LuaApi resolve_from_main_module();
        [[nodiscard]] bool is_complete() const;
    };
}
