#pragma once

#include <string_view>
#include <windows.h>

namespace platform
{
    template <typename Function>
    Function find_main_module_export(const std::string_view name)
    {
        const auto module = GetModuleHandle(nullptr);
        if (module == nullptr)
        {
            return nullptr;
        }

        const auto base = reinterpret_cast<const BYTE*>(module);
        const auto dos_header = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
        if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
        {
            return nullptr;
        }

        const auto nt_headers = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + dos_header->e_lfanew);
        if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
        {
            return nullptr;
        }

        const auto& directory = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
        if (directory.VirtualAddress == 0)
        {
            return nullptr;
        }

        const auto exports = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(base + directory.VirtualAddress);
        const auto functions = reinterpret_cast<const DWORD*>(base + exports->AddressOfFunctions);
        const auto names = reinterpret_cast<const DWORD*>(base + exports->AddressOfNames);
        const auto ordinals = reinterpret_cast<const WORD*>(base + exports->AddressOfNameOrdinals);

        for (DWORD i = 0; i < exports->NumberOfNames; ++i)
        {
            const auto exported_name = reinterpret_cast<const char*>(base + names[i]);
            if (name == exported_name)
            {
                return reinterpret_cast<Function>(const_cast<BYTE*>(base) + functions[ordinals[i]]);
            }
        }
        return nullptr;
    }
}
