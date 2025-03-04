//
// Created by Aleksej on 28.02.2025.
//

#ifndef LUA_FUNCTIONS_H
#define LUA_FUNCTIONS_H
#include <map>
#include <vector>

typedef int (__cdecl*tlua_loadbuffer)(int *lua_State, const char *buff, size_t sz, const char *name);

typedef int (__cdecl*tlua_pcall)(int *lua_state, int nargs, int nresults, int msgh);

typedef int (__cdecl*tlua_loadstring)(int *lua_state, const char *s);

typedef void (__cdecl*tlua_settop)(int *lua_state, int index);

typedef int (__cdecl*tlua_isnumber)(int *lua_state, int index);

typedef double (__cdecl*tlua_tonumber)(int *lua_state, int index);

typedef const char * (__cdecl*tlua_tolstring)(int *lua_state, int index, size_t *len);

typedef void (__cdecl*tlua_pushnil)(int *lua_state);

typedef int (__cdecl*tlua_next)(int *lua_state, int index);

typedef int (__cdecl*tlua_getfield)(int *lua_state, int index, const char *k);

class lua_functions
{
private:
    std::map<void *, std::vector<char> > _trampolines{};

public:
    tlua_loadbuffer load_buffer;
    tlua_pcall pcall;
    tlua_loadstring loadstring;
    tlua_settop settop;
    tlua_isnumber isnumber;
    tlua_tonumber tonumber;
    tlua_tolstring tolstring;
    tlua_pushnil pushnil;
    tlua_next next;
    tlua_getfield getfield;

    tlua_loadbuffer load_buffer_trampoline = nullptr;
    tlua_pcall pcall_trampoline = nullptr;

    lua_functions(
        tlua_loadbuffer load_buffer,
        tlua_pcall pcall,
        tlua_loadstring loadstring,
        tlua_settop settop,
        tlua_isnumber isnumber,
        tlua_tonumber tonumber,
        tlua_tolstring tolstring,
        tlua_pushnil pushnil,
        tlua_next next,
        tlua_getfield getfield);

    void install_load_buffer_hook(tlua_loadbuffer hook);

    void install_pcall_hook(tlua_pcall hook);
};


#endif //LUA_FUNCTIONS_H
