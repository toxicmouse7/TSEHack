//
// Created by Aleksej on 28.02.2025.
//

#include "lua_functions.h"

#include <windows.h>

lua_functions::lua_functions(
    tlua_loadbuffer load_buffer,
    tlua_pcall pcall,
    tlua_loadstring loadstring,
    tlua_settop settop,
    tlua_isnumber isnumber,
    tlua_tonumber tonumber,
    tlua_tolstring tolstring,
    tlua_pushnil pushnil,
    tlua_next next,
    tlua_getfield getfield)
{
    this->load_buffer = load_buffer;
    this->pcall = pcall;
    this->loadstring = loadstring;
    this->settop = settop;
    this->isnumber = isnumber;
    this->tonumber = tonumber;
    this->tolstring = tolstring;
    this->pushnil = pushnil;
    this->next = next;
    this->getfield = getfield;
}

void lua_functions::install_load_buffer_hook(tlua_loadbuffer hook)
{
    std::vector<char> trampoline(12);
    memcpy(trampoline.data(), this->load_buffer, 7);

    char jmp[7];
    jmp[0] = static_cast<char>(0x90);
    jmp[1] = static_cast<char>(0x90);
    jmp[2] = static_cast<char>(0xE9);


    auto relativeAddress = reinterpret_cast<uintptr_t>(hook) - reinterpret_cast<uintptr_t>(this->load_buffer) - 7;
    memcpy(&jmp[3], &relativeAddress, 4);

    DWORD oldProtect = 0;
    VirtualProtect(this->load_buffer, 7, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(this->load_buffer, jmp, 7);

    char jmp2[5];
    jmp2[0] = static_cast<char>(0xE9);

    auto relAddress2 = reinterpret_cast<uintptr_t>(this->load_buffer) + 7
                       - reinterpret_cast<uintptr_t>(&trampoline[7]) - 5;
    memcpy(&jmp2[1], &relAddress2, 4);

    memcpy(&trampoline[7], jmp2, 5);

    VirtualProtect(trampoline.data(), 12, PAGE_EXECUTE_READWRITE, &oldProtect);

    _trampolines[this->load_buffer] = std::move(trampoline);
    this->load_buffer_trampoline = reinterpret_cast<tlua_loadbuffer>(_trampolines[this->load_buffer].data());
}

void lua_functions::install_pcall_hook(tlua_pcall hook)
{
    std::vector<char> trampoline(13);
    memcpy(trampoline.data(), this->pcall, 8);

    char jmp[8];
    jmp[0] = static_cast<char>(0x90);
    jmp[1] = static_cast<char>(0x90);
    jmp[2] = static_cast<char>(0x90);
    jmp[3] = static_cast<char>(0xE9);


    auto relativeAddress = reinterpret_cast<uintptr_t>(hook) - reinterpret_cast<uintptr_t>(this->pcall) - 8;
    memcpy(&jmp[4], &relativeAddress, 4);

    DWORD oldProtect = 0;
    VirtualProtect(this->pcall, 8, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(this->pcall, jmp, 8);

    char jmp2[5];
    jmp2[0] = static_cast<char>(0xE9);

    auto relAddress2 = reinterpret_cast<uintptr_t>(this->pcall) + 8
                       - reinterpret_cast<uintptr_t>(&trampoline[8]) - 5;
    memcpy(&jmp2[1], &relAddress2, 4);

    memcpy(&trampoline[8], jmp2, 5);

    VirtualProtect(trampoline.data(), 13, PAGE_EXECUTE_READWRITE, &oldProtect);

    _trampolines[this->pcall] = std::move(trampoline);
    this->pcall_trampoline = reinterpret_cast<tlua_pcall>(_trampolines[this->pcall].data());
}
