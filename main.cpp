#include <iostream>
#include <vector>
#include <windows.h>
#include <tlhelp32.h>

// Prototipos de Funções
DWORD_PTR GetProcessId(const wchar_t* szProcessName);
uintptr_t GetModuleBaseAddress(const wchar_t* szModuleName, DWORD dwProcessId);
uintptr_t FindDynamicAddress(HANDLE hProcess, uintptr_t uiModuleBaseAddress, const std::vector<int>& offsets);
void KeyUp(BYTE vk);

// offsets
std::vector<int> Gold_offset = {0x04CAD688, 0x110, 0x28, 0x18, 0x50, 0x78};

int main() {
    SetConsoleTitleW(L"Hack Vampire Survivors");

    // Get Process ID
    DWORD_PTR dwPID = GetProcessId(L"VampireSurvivors.exe");
    if (!dwPID) {
        return EXIT_FAILURE;
    }

    // Get Module Base Address
    uintptr_t uiBA = GetModuleBaseAddress(L"GameAssembly.dll", dwPID);
    if (!uiBA) {
        return EXIT_FAILURE;
    }

    // Attach Handle to the Process
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
    if (!hProcess) {
        return EXIT_FAILURE;
    }

    // Print Process Info
    std::cout << "Nome do processo: VampireSurvivors.exe\n"
              << "Developer - by Swth Saw Theme\n"
              << "[F2] - Para adicionar 500 de Gold\n"
              << "[F1 - Para sair...]\n\n";

    for (;;) {
        float buffer;

        if (GetAsyncKeyState(VK_F2)) {
            KeyUp(VK_F2);
            uintptr_t address = FindDynamicAddress(hProcess, uiBA, Gold_offset);
            ReadProcessMemory(hProcess, (LPCVOID)address, &buffer, sizeof(buffer), 0);
            buffer += 500.0;
            std::cout << "\rGOLD: " << buffer << std::flush;
            WriteProcessMemory(hProcess, (LPVOID)address, &buffer, sizeof(buffer), 0);
            
        }
        if (GetAsyncKeyState(VK_F1)) {
            break;
        }
    }
    CloseHandle(hProcess);
    return EXIT_SUCCESS;
}

DWORD_PTR GetProcessId(const wchar_t* szProcessName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    DWORD_PTR dwProcessId = 0;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32);

    if (Process32First(hSnapshot, &pe32)) {
        do {
            wchar_t exeFileW[MAX_PATH];
            MultiByteToWideChar(CP_ACP, 0, pe32.szExeFile, -1, exeFileW, MAX_PATH);
            if (!_wcsicmp(szProcessName, exeFileW)) {
                dwProcessId = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return dwProcessId;
}

uintptr_t GetModuleBaseAddress(const wchar_t* szModuleName, DWORD dwProcessId) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwProcessId);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    uintptr_t uiBaseAddress = 0;
    MODULEENTRY32 me32;
    me32.dwSize = sizeof(me32);

    if (Module32First(hSnapshot, &me32)) {
        do {
            wchar_t moduleNameW[MAX_PATH];
            MultiByteToWideChar(CP_ACP, 0, me32.szModule, -1, moduleNameW, MAX_PATH);
            if (!_wcsicmp(szModuleName, moduleNameW)) {
                uiBaseAddress = (uintptr_t)me32.modBaseAddr;
                break;
            }
        } while (Module32Next(hSnapshot, &me32));
    }

    CloseHandle(hSnapshot);
    return uiBaseAddress;
}

uintptr_t FindDynamicAddress(HANDLE hProcess, uintptr_t uiModuleBaseAddress, const std::vector<int>& Offsets) {
    uintptr_t uiDynamicAddress = uiModuleBaseAddress + Offsets[0];
    for (int i = 1; i < Offsets.size(); i++) {
        ReadProcessMemory(hProcess, (LPCVOID)uiDynamicAddress, &uiDynamicAddress, sizeof(uiDynamicAddress), 0);
        uiDynamicAddress += Offsets[i];
    }
    return uiDynamicAddress;
}

void KeyUp(BYTE vk) {
    keybd_event(vk, MapVirtualKey(vk, MAPVK_VK_TO_VSC), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
}