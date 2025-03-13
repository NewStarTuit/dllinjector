#include <windows.h>
#include <string>
#include <iostream>

bool inject(DWORD proc_id, const std::wstring& dll_path) {
    // Get a handle to the target process
    HANDLE hProcess = OpenProcess(
        PROCESS_ALL_ACCESS,  // Request full access to the process
        FALSE,               // Don't inherit handle
        proc_id              // Target process ID
    );

    if (hProcess == NULL) {
        std::cerr << "Failed to open process with ID: " << proc_id << std::endl;
        std::cerr << "Error code: " << GetLastError() << std::endl;
        return false;
    }

    // Allocate memory in the target process for the DLL path
    SIZE_T dll_path_size = (dll_path.length() + 1) * sizeof(wchar_t);
    LPVOID pRemotePath = VirtualAllocEx(
        hProcess,           // Handle to the target process
        NULL,               // Let the system decide where to allocate
        dll_path_size,      // Size of allocation
        MEM_COMMIT,         // Commit the memory
        PAGE_READWRITE      // Memory protection
    );

    if (pRemotePath == NULL) {
        std::cerr << "Failed to allocate memory in target process." << std::endl;
        std::cerr << "Error code: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    // Write the DLL path to the allocated memory
    BOOL result = WriteProcessMemory(
        hProcess,                        // Handle to the target process
        pRemotePath,                     // Remote memory address to write to
        dll_path.c_str(),                // Data to write (DLL path)
        dll_path_size,                   // Size of data to write
        NULL                             // Don't need bytes written
    );

    if (result == FALSE) {
        std::cerr << "Failed to write DLL path to target process memory." << std::endl;
        std::cerr << "Error code: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Get the address of LoadLibraryW (for wide strings) function from kernel32.dll
    HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");
    if (hKernel32 == NULL) {
        std::cerr << "Failed to get handle to kernel32.dll" << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    LPVOID pLoadLibrary = (LPVOID)GetProcAddress(hKernel32, "LoadLibraryW");
    if (pLoadLibrary == NULL) {
        std::cerr << "Failed to get address of LoadLibraryW function." << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Create a remote thread in the target process to call LoadLibraryW
    HANDLE hThread = CreateRemoteThread(
        hProcess,           // Handle to the target process
        NULL,               // Default security descriptor
        0,                  // Default stack size
        (LPTHREAD_START_ROUTINE)pLoadLibrary,  // Thread function (LoadLibraryW)
        pRemotePath,        // Parameter to thread function (DLL path)
        0,                  // Run thread immediately
        NULL                // Don't need thread ID
    );

    if (hThread == NULL) {
        std::cerr << "Failed to create remote thread." << std::endl;
        std::cerr << "Error code: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Wait for the thread to finish execution
    WaitForSingleObject(hThread, INFINITE);

    // Get the thread exit code (handle to the loaded DLL)
    DWORD exitCode = 0;
    GetExitCodeThread(hThread, &exitCode);

    // Clean up
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    // If exitCode is 0, the DLL failed to load
    if (exitCode == 0) {
        std::cerr << "DLL failed to load in the target process." << std::endl;
        return false;
    }

    std::cout << "DLL successfully injected into process ID: " << proc_id << std::endl;
    return true;
}

// Convert string to wstring
std::wstring string_to_wstring(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

int main(int argc, char* argv[]) {
    // Check if correct number of arguments is provided
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <process_id> <dll_path>" << std::endl;
        std::cout << "Example: " << argv[0] << " 1234 C:\\path\\to\\your\\dll.dll" << std::endl;
        return 1;
    }

    // Parse process ID from command line argument
    DWORD processId;
    try {
        processId = static_cast<DWORD>(std::stoul(argv[1]));
    }
    catch (const std::exception& e) {
        std::cerr << "Invalid process ID: " << argv[1] << std::endl;
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    // Get DLL path from command line argument and convert to wstring
    std::string dllPathStr = argv[2];
    std::wstring dllPath = string_to_wstring(dllPathStr);

    std::cout << "Target Process ID: " << processId << std::endl;
    std::cout << "DLL Path: " << dllPathStr << std::endl;

    // Call the inject function
    if (inject(processId, dllPath)) {
        std::cout << "Injection successful!" << std::endl;
        return 0;
    }
    else {
        std::cout << "Injection failed!" << std::endl;
        return 1;
    }
}