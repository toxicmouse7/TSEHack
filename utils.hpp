//
// Created by Aleksej on 28.02.2025.
//
#pragma once

#include <string>
#include <windows.h>

class utils
{
public:
    template <typename TFunc>
    static TFunc get_func_from_export(std::string&& func_name)
    {
        auto hModule = GetModuleHandle(nullptr);

        auto pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
        auto pNtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
            reinterpret_cast<BYTE *>(hModule) + pDosHeader->e_lfanew);

        auto pExportDirectory = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(
            reinterpret_cast<BYTE *>(hModule) +
            pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].
            VirtualAddress);

        auto pFunctions = reinterpret_cast<DWORD *>(
            reinterpret_cast<BYTE *>(hModule) + pExportDirectory->AddressOfFunctions);
        auto pNames = reinterpret_cast<DWORD *>(
            reinterpret_cast<BYTE *>(hModule) + pExportDirectory->AddressOfNames);
        auto pNameOrdinals = reinterpret_cast<WORD *>(
            reinterpret_cast<BYTE *>(hModule) + pExportDirectory->AddressOfNameOrdinals);

        for (DWORD i = 0; i < pExportDirectory->NumberOfNames; i++)
        {
            auto pFunctionName = reinterpret_cast<const char *>(reinterpret_cast<BYTE *>(hModule) + pNames[i]);
            if (func_name == pFunctionName)
            {
                return reinterpret_cast<TFunc>(reinterpret_cast<BYTE *>(hModule) + pFunctions[pNameOrdinals[i]]);
            }
        }

        return nullptr;
    }
};
