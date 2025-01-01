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

void Spinner(std::atomic<bool>& running, const char* processName) {
    const char spinnerChars[] = { '|', '/', '-', '\\' };

    int spinnerIdx = 0;

    while (running) {
        std::cout << "\rWaiting for: " << processName << " "
            << spinnerChars[spinnerIdx++ % 4];
        std::cout.flush();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "\r" << std::string(50, ' ') << "\r";
}

DWORD GetProcessByName(const char* lpProcessName)
{
    char lpCurrentProcessName[255];

    PROCESSENTRY32 ProcList{};
    ProcList.dwSize = sizeof(ProcList);

    const HANDLE hProcList = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcList == INVALID_HANDLE_VALUE)
        return -1;

    if (!Process32First(hProcList, &ProcList)) {
        CloseHandle(hProcList);
        return -1;
    }

    wcstombs_s(nullptr, lpCurrentProcessName, ProcList.szExeFile, 255);

    if (lstrcmpA(lpCurrentProcessName, lpProcessName) == 0) {
        CloseHandle(hProcList);
        return ProcList.th32ProcessID;
    }

    while (Process32Next(hProcList, &ProcList))
    {
        wcstombs_s(nullptr, lpCurrentProcessName, ProcList.szExeFile, 255);

        if (lstrcmpA(lpCurrentProcessName, lpProcessName) == 0) {
            CloseHandle(hProcList);
            return ProcList.th32ProcessID;
        }
    }

    CloseHandle(hProcList);
    return -1;
}


int main(const int argc, char* argv[])
{
    const char* lpDLLName = "skeet.dll";
    const char* lpProcessName = "csgo.exe";
    char lpFullDLLPath[MAX_PATH];
    
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE)
        return -1;

    std::atomic<bool> spinning(true);
    std::thread spinnerThread(Spinner, std::ref(spinning), lpProcessName);

    DWORD dwProcessID = (DWORD)-1;

    while (dwProcessID == (DWORD)-1)
    {
        dwProcessID = GetProcessByName(lpProcessName);
        if (dwProcessID == (DWORD)-1)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    spinning = false;
    spinnerThread.join();

    SetColor(10); 
    std::cout << "\rProcess found!                              \n";
    std::cout << "Process ID: " << dwProcessID << "\n\n";

    SetColor(15); 

    const DWORD dwFullPathResult = GetFullPathNameA(lpDLLName, MAX_PATH, lpFullDLLPath, nullptr);
    if (dwFullPathResult == 0)
    {
        SetColor(12); 
        printf("Failed to get DLL, Make sure its in the same path as loader.\n");
        SetColor(15);
        return -1;
    }

    const HANDLE hTargetProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);
    if (hTargetProcess == INVALID_HANDLE_VALUE || hTargetProcess == nullptr)
    {
        SetColor(12); 
        printf("An error occurred when trying to open the target process.\n");
        SetColor(15);
        return -1;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    LPVOID allocatedAddress1 = VirtualAllocEx(hTargetProcess, (LPVOID)0x43310000, 0x2FC000u, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (allocatedAddress1 == nullptr)
    {
        SetColor(12); 
        printf("Failed to allocate memory at 0x43310000.\n");
        SetColor(15);
        CloseHandle(hTargetProcess);
        return -1;
    }

    LPVOID allocatedAddress2 = VirtualAllocEx(hTargetProcess, nullptr, 0x1000u, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (allocatedAddress2 == nullptr)
    {
        SetColor(12); 
        printf("Failed to allocate additional memory.\n");
        SetColor(15);
        VirtualFreeEx(hTargetProcess, allocatedAddress1, 0, MEM_RELEASE);
        CloseHandle(hTargetProcess);
        return -1;
    }

    SetColor(11); 
    printf("Allocating memory for DLL path...\n");
    SetColor(15);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    const LPVOID lpPathAddress = VirtualAllocEx(hTargetProcess, nullptr, lstrlenA(lpFullDLLPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (lpPathAddress == nullptr)
    {
        SetColor(12); 
        printf("An error occurred when trying to allocate memory in the target process.\n");
        SetColor(15);
        VirtualFreeEx(hTargetProcess, allocatedAddress1, 0, MEM_RELEASE);
        VirtualFreeEx(hTargetProcess, allocatedAddress2, 0, MEM_RELEASE);
        CloseHandle(hTargetProcess);
        return -1;
    }

    SetColor(10); 
    printf("Memory allocated for DLL path at 0x%p\n", lpPathAddress);
    SetColor(15);

    SetColor(11);
    printf("Writing DLL path to target process memory...\n");
    SetColor(15);
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); 

    const DWORD dwWriteResult = WriteProcessMemory(hTargetProcess, lpPathAddress, lpFullDLLPath, lstrlenA(lpFullDLLPath) + 1, nullptr);
    if (dwWriteResult == 0)
    {
        SetColor(12); 
        printf("An error occurred when trying to write the DLL path in the target process.\n");
        SetColor(15);
        VirtualFreeEx(hTargetProcess, allocatedAddress1, 0, MEM_RELEASE);
        VirtualFreeEx(hTargetProcess, allocatedAddress2, 0, MEM_RELEASE);
        VirtualFreeEx(hTargetProcess, lpPathAddress, 0, MEM_RELEASE);
        CloseHandle(hTargetProcess);
        return -1;
    }

    SetColor(10); 
    printf("DLL path written successfully.\n");
    SetColor(15);

    const HMODULE hModule = GetModuleHandleA("kernel32.dll");
    if (hModule == INVALID_HANDLE_VALUE || hModule == nullptr)
    {
        SetColor(12);
        printf("An error occurred when trying to get \"kernel32.dll\" handle.\n");
        SetColor(15);

        VirtualFreeEx(hTargetProcess, allocatedAddress1, 0, MEM_RELEASE);
        VirtualFreeEx(hTargetProcess, allocatedAddress2, 0, MEM_RELEASE);
        VirtualFreeEx(hTargetProcess, lpPathAddress, 0, MEM_RELEASE);
        CloseHandle(hTargetProcess);
        return -1;
    }

    const FARPROC lpFunctionAddress = GetProcAddress(hModule, "LoadLibraryA");
    if (lpFunctionAddress == nullptr)
    {
        SetColor(12); 
        printf("An error occurred when trying to get \"LoadLibraryA\" address.\n");
        SetColor(15);

        VirtualFreeEx(hTargetProcess, allocatedAddress1, 0, MEM_RELEASE);
        VirtualFreeEx(hTargetProcess, allocatedAddress2, 0, MEM_RELEASE);
        VirtualFreeEx(hTargetProcess, lpPathAddress, 0, MEM_RELEASE);
        CloseHandle(hTargetProcess);
        return -1;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    const HANDLE hThreadCreationResult = CreateRemoteThread(
        hTargetProcess,
        nullptr,
        0,
        (LPTHREAD_START_ROUTINE)lpFunctionAddress,
        lpPathAddress,
        0,
        nullptr
    );

    if (hThreadCreationResult == nullptr)
    {
        SetColor(12);
        printf("An error occurred when trying to create the thread in the target process.\n");
        SetColor(15);
        VirtualFreeEx(hTargetProcess, allocatedAddress1, 0, MEM_RELEASE);
        VirtualFreeEx(hTargetProcess, allocatedAddress2, 0, MEM_RELEASE);
        VirtualFreeEx(hTargetProcess, lpPathAddress, 0, MEM_RELEASE);
        CloseHandle(hTargetProcess);
        return -1;
    }

    SetColor(10); 
    printf("DLL Injected Successfully!\n");
    SetColor(15);

    printf("Raze gf Fucked by Spencer!\n");
    SetColor(15);

   // CloseHandle(hThreadCreationResult);
   // CloseHandle(hTargetProcess);

    // VirtualFreeEx(hTargetProcess, allocatedAddress1, 0, MEM_RELEASE);
    // VirtualFreeEx(hTargetProcess, allocatedAddress2, 0, MEM_RELEASE);
    // VirtualFreeEx(hTargetProcess, lpPathAddress, 0, MEM_RELEASE);

    return 0;
}
