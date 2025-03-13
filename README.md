# DLL Injector

A simple C++ utility for injecting DLL files into running processes on Windows systems.

## Overview

This DLL injector allows you to load a custom DLL into any running process by specifying the process ID and the path to the DLL file. The injection is performed using the Windows API functions to create a remote thread that calls `LoadLibraryW` in the target process.

## Features

- Command-line interface for easy automation
- Detailed error reporting
- Clean resource management
- Support for Unicode paths

## Requirements

- Windows operating system
- Visual Studio or another C++ compiler with Windows SDK
- Administrator privileges (usually required for process manipulation)

## Building

To build the DLL injector, use the following commands:

```bash
# Using Visual Studio Developer Command Prompt
cl /EHsc /W4 DLL-Injector.cpp /link /OUT:DLL-Injector.exe

# Using MinGW
g++ -o DLL-Injector.exe DLL-Injector.cpp -lpsapi
```

## Usage

```
DLL-Injector.exe <process_id> <dll_path>
```

### Parameters

- `<process_id>`: The ID of the target process (can be found using Task Manager)
- `<dll_path>`: Full path to the DLL file you want to inject

### Example

```
DLL-Injector.exe 1234 C:\path\to\your\dll.dll
```

## How It Works

1. Opens a handle to the target process
2. Allocates memory within the target process for the DLL path
3. Writes the DLL path to the allocated memory
4. Creates a remote thread in the target process
5. The remote thread calls LoadLibraryW with the DLL path as a parameter
6. The DLL is loaded into the target process's address space

## Error Codes

If the injection fails, the program will return exit code 1 and display detailed error information. Some common Windows API error codes you might encounter:

- 5: Access denied (try running with administrator privileges)
- 87: Invalid parameter
- 6: Invalid handle
- 998: Invalid access to memory location

## Legal Notice

This tool is provided for educational and legitimate software testing purposes only. Unauthorized modification of software may violate terms of service or local laws. The user assumes all responsibility for how this tool is used.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
