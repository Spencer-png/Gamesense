//Loader made by Spencer 
// Discord: " not_spencer " for any problems 

#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

void SetColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

DWORD GetProcessByName(const char* lpProcessName)
{
    char lpCurrentProcessName[255];

    PROCESSENTRY32 ProcList{};
    ProcList.dwSize = sizeof(ProcList);

    const HANDLE hProcList = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcList == INVALID_HANDLE_VALUE)
        return -1;

    if (!Process32First(hProcList, &ProcList))
        return -1;

    wcstombs_s(nullptr, lpCurrentProcessName, ProcList.szExeFile, 255);

    if (lstrcmpA(lpCurrentProcessName, lpProcessName) == 0)
        return ProcList.th32ProcessID;

    while (Process32Next(hProcList, &ProcList))
    {
        wcstombs_s(nullptr, lpCurrentProcessName, ProcList.szExeFile, 255);

        if (lstrcmpA(lpCurrentProcessName, lpProcessName) == 0)
            return ProcList.th32ProcessID;
    }

    return -1;
}

//DWORD LaunchCSGO(const char* lpProcessName)
//{
//    std::wstring executablePath = L"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\csgo.exe";
//    std::wstring arguments = L" -steam -insecure";
//    std::wstring commandLine = executablePath + arguments;
//
//    STARTUPINFOW si = { 0 };
//    si.cb = sizeof(si);
//    PROCESS_INFORMATION pi = { 0 };
//
//    if (!CreateProcessW(
//        nullptr,
//        &commandLine[0],
//        nullptr,
//        nullptr,
//        FALSE,
//        0,
//        nullptr,
//        nullptr,
//        &si,
//        &pi
//    )) {
//        DWORD error = GetLastError();
//        std::wcerr << L"Failed to launch CS:GO. Error: " << error << std::endl;
//
//        if (error == ERROR_FILE_NOT_FOUND) {
//            std::wcerr << L"Executable not found. Check the path: " << executablePath << std::endl;
//        }
//        else if (error == ERROR_ACCESS_DENIED) {
//            std::wcerr << L"Access denied. Try running as Administrator." << std::endl;
//        }
//    }
//    else 
//    {
//
//        printf("Launching: %s...\n", lpProcessName);
//        printf("Please wait");
//
//        //WaitForSingleObject(pi.hProcess, INFINITE);
//
//        CloseHandle(pi.hProcess);
//        CloseHandle(pi.hThread);
//
//    }
//}

int main(const int argc, char* argv[])
{
    const char* lpDLLName = "skeet.dll";
    const char* lpProcessName = "csgo.exe";
    char lpFullDLLPath[MAX_PATH];

    SetColor(15);
    std::cout << "\rMade by Spencer.                              \n";
    std::cout << "\r                                              \n";

    printf("Please open - % s\n", lpProcessName);

    DWORD dwProcessID = (DWORD)-1;

    while (dwProcessID == (DWORD)-1)
    {
        dwProcessID = GetProcessByName(lpProcessName);
        if (dwProcessID == (DWORD)-1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    printf("Process found!\n");
    printf("Process ID: %i\n\n", (int)dwProcessID);

    const DWORD dwFullPathResult = GetFullPathNameA(lpDLLName, MAX_PATH, lpFullDLLPath, nullptr);
    if (dwFullPathResult == 0)
    {
        printf("An error occurred when trying to get the full path of the DLL.\n");
        return -1;
    }

    const HANDLE hTargetProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);
    if (hTargetProcess == INVALID_HANDLE_VALUE)
    {
        printf("An error occurred when trying to open the target process.\n");
        return -1;
    }

    printf("CS:GO opened successfully.\n");

    VirtualAllocEx(hTargetProcess, (LPVOID)0x43310000, 0x2FC000u, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); // for skeet
    VirtualAllocEx(hTargetProcess, 0, 0x1000u, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); // for skeet

    const LPVOID lpPathAddress = VirtualAllocEx(hTargetProcess, nullptr, lstrlenA(lpFullDLLPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (lpPathAddress == nullptr)
    {
        printf("An error occurred when trying to allocate memory in the target process.\n");
        return -1;
    }

    printf("Memory allocated at 0x%X\n", (UINT)(uintptr_t)lpPathAddress);

    const DWORD dwWriteResult = WriteProcessMemory(hTargetProcess, lpPathAddress, lpFullDLLPath, lstrlenA(lpFullDLLPath) + 1, nullptr);
    if (dwWriteResult == 0)
    {
        printf("An error occurred when trying to write the DLL path in the target process.\n");
        return -1;
    }

    printf("DLL path written successfully.\n");

    const HMODULE hModule = GetModuleHandleA("kernel32.dll");
    if (hModule == INVALID_HANDLE_VALUE || hModule == nullptr)
        return -1;

    const FARPROC lpFunctionAddress = GetProcAddress(hModule, "LoadLibraryA");
    if (lpFunctionAddress == nullptr)
    {
        printf("An error occurred when trying to get \"LoadLibraryA\" address.\n");
        return -1;
    }

    printf("LoadLibraryA address at 0x%X\n", (UINT)(uintptr_t)lpFunctionAddress);

    const HANDLE hThreadCreationResult = CreateRemoteThread(hTargetProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)lpFunctionAddress, lpPathAddress, 0, nullptr);
    if (hThreadCreationResult == INVALID_HANDLE_VALUE)
    {
        printf("An error occurred when trying to create the thread in the target process.\n");
        return -1;
    }

    printf("DLL Injected!\n");
    printf("I FUCKED RAZE FEMBOY GF!\n");

    return 0;
}