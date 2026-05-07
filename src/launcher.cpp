#include <fstream>
#include <iostream>

#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // Redirect std::wcerr to a file
    static std::wofstream log("launcher.log");
    std::wcerr.rdbuf(log.rdbuf());

    // Find GenshinImpact.exe & DLL in the current directory
    wchar_t cwd[MAX_PATH];
    if (!GetCurrentDirectoryW(MAX_PATH, cwd)) {
        std::wcerr << "Failed to get current directory" << '\n';
        return 1;
    }

    std::wcerr << "Current directory: " << cwd << '\n';

    wchar_t exePath[MAX_PATH];
    if (!SearchPathW(cwd, L"GenshinImpact.exe", nullptr, MAX_PATH, exePath, nullptr)) {
        std::wcerr << "Failed to find GenshinImpact.exe in the current directory" << '\n';
        return 1;
    }

    std::wcerr << "Found GenshinImpact.exe at: " << exePath << '\n';

    wchar_t dllPath[MAX_PATH];
    if (!SearchPathW(cwd, L"GenshinImpactUnlocker.dll", nullptr, MAX_PATH, dllPath, nullptr)) {
        std::wcerr << "Failed to find GenshinImpactUnlocker.dll in the current directory" << '\n';
        return 1;
    }

    std::wcerr << "Found GenshinImpactUnlocker.dll at: " << dllPath << '\n';

    // Launch Genshin Impact
    wchar_t cmdline[MAX_PATH];
    swprintf_s(cmdline, MAX_PATH, L"\"%s\" -screen-fullscreen 0 -monitor 1", exePath);

    STARTUPINFOW si{ .cb = sizeof(STARTUPINFOW) };
    PROCESS_INFORMATION pi{};

    if (!CreateProcessW(exePath, cmdline, 0, 0, FALSE, 0, 0, cwd, &si, &pi)) {
        std::wcerr << "Failed to launch Genshin Impact" << '\n';
        return 1;
    }

    // Inject unlocker DLL into the process
    LPVOID remoteVa{VirtualAllocEx(
        pi.hProcess,
        0,
        0x1000,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    )};
    if (!remoteVa) {
        std::wcerr << "Failed to allocate memory in remote process" << '\n';
        return 1;
    }

    size_t dllPathSize{(wcslen(dllPath) + 1) * sizeof(wchar_t)};
    if (!WriteProcessMemory(pi.hProcess, remoteVa, dllPath, dllPathSize, nullptr)) {
        std::wcerr << "Failed to write DLL path to remote process" << '\n';
        return 1;
    }

    LPTHREAD_START_ROUTINE llAddr{reinterpret_cast<LPTHREAD_START_ROUTINE>(
        reinterpret_cast<void*>(
            GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW")
        )
    )};
    HANDLE thread{CreateRemoteThread(pi.hProcess, 0, 0, llAddr, remoteVa, 0, 0)};
    if (!thread) {
        std::wcerr << "Failed to create remote thread in remote process" << '\n';
        return 1;
    }

    std::wcerr << "Successfully injected DLL into Genshin Impact" << '\n';

    // Wait for the remote thread to finish
    WaitForSingleObject(thread, INFINITE);

    std::wcerr << "DLL injection complete" << '\n';

    CloseHandle(thread);
    VirtualFreeEx(pi.hProcess, remoteVa, 0, MEM_RELEASE);

    std::wcerr << "Waiting for Genshin Impact to exit..." << '\n';

    // Wait forever
    WaitForSingleObject(pi.hProcess, INFINITE);

    std::wcerr << "Genshin Impact has exited" << '\n';

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}
