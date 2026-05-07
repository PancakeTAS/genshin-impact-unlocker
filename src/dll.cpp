#include <algorithm>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <span>
#include <string_view>

#include <libloaderapi.h>
#include <windows.h>

EXTERN_C NTSTATUS __stdcall LdrAddRefDll(ULONG Flags, PVOID BaseAddress);

DWORD __stdcall ThreadProc(LPVOID lpParameter)
{
    std::optional<std::span<uint8_t>> il2cpp;
    std::optional<uint8_t*> address;
    int32_t* framerate{nullptr};

    // Increment reference count
    LdrAddRefDll(1, lpParameter);

    // Find IL2CPP section in NT header
    const auto imageBase{reinterpret_cast<uint8_t*>(GetModuleHandleW(nullptr))};

    const auto dosHeader{reinterpret_cast<IMAGE_DOS_HEADER*>(imageBase)};
    const auto ntHeaders{reinterpret_cast<IMAGE_NT_HEADERS*>(imageBase + dosHeader->e_lfanew)};

    const auto sectionCount{ntHeaders->FileHeader.NumberOfSections};
    std::span<IMAGE_SECTION_HEADER> sections{IMAGE_FIRST_SECTION(ntHeaders), sectionCount};

    for (const auto& section : sections) {
        const std::string_view name{reinterpret_cast<const char*>(section.Name)};
        if (name != "il2cpp")
            continue;

        il2cpp.emplace(imageBase + section.VirtualAddress, section.Misc.VirtualSize);
        break;
    }

    if (!il2cpp.has_value()) {
        std::wcerr << "Failed to find IL2CPP section in NT header" << '\n';
        return FALSE;
    }

    // Scan for specific pattern in IL2CPP section
    constexpr std::array<uint8_t, 6> PATTERN{0xB9, 0x3C, 0x00, 0x00, 0x00, 0xE8};
    const auto SEARCHER{std::boyer_moore_horspool_searcher(PATTERN.begin(), PATTERN.end())};

    auto it{il2cpp->begin()};
    while ((it = std::search(it, il2cpp->end(), SEARCHER)) != il2cpp->end()) {
        uint8_t* ptr{&*it};
        it++;

        uint8_t* rip{ptr + 5};
        const auto disp{*reinterpret_cast<const int32_t*>(rip + 1)};
        const auto dest{rip + disp + 5};
        if (*dest == 0xE9) {
            address.emplace(rip);
            break;
        }
    }

    if (!address.has_value()) {
        std::wcerr << "Failed to find pattern in IL2CPP section" << '\n';
        return FALSE;
    }

    // Calculate pointer to framerate variable
    uint8_t* rip{*address};
    while (rip[0] == 0xE8 || rip[0] == 0xE9) {
        const auto disp{*reinterpret_cast<const int32_t*>(rip + 1)};
        rip += disp + 5;
    }

    const auto disp{*reinterpret_cast<const int32_t*>(rip + 2)};
    framerate = reinterpret_cast<int32_t*>(rip + disp + 6);

    std::wcerr << "Successfully found fps cap at address: "
        << static_cast<void*>(framerate) << '\n';
    std::wcerr << "Original fps cap: " << *framerate << '\n';

    // Uncap framerate periodically
    while (true) {
        *framerate = 120;
        Sleep(62);
    }

    return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hInstance, DWORD fdwReason, LPVOID) {
    if (hInstance)
        DisableThreadLibraryCalls(hInstance);

    if (fdwReason == DLL_PROCESS_ATTACH) {
        // Redirect std::wcerr to a file
        static std::wofstream log("game.log");
        std::wcerr.rdbuf(log.rdbuf());

        // Create a thread to run the unlocker logic
        const auto thread{CreateThread(nullptr, 0, ThreadProc, hInstance, 0, nullptr)};
        if (!thread) {
            std::wcerr << "Failed to create thread in DllMain" << '\n';
            return FALSE;
        }

        CloseHandle(thread);
    }

    return TRUE;
}
