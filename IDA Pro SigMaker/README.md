# SigMaker

IDA Pro plugin for generating and searching byte signatures.

## Build with CMake

This project now builds through CMake instead of the legacy Visual Studio project.

### Requirements

- CMake 3.21 or newer
- A C++23-capable compiler
- The Hex-Rays IDA SDK

### macOS

Build for the native host architecture:

```sh
export IDASDK=/Users/student/Documents/IDASDK
cmake -S . -B build
cmake --build build
```

Build explicitly for Apple Silicon:

```sh
export IDASDK=/Users/student/Documents/IDASDK
cmake -S . -B build-arm64 -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build-arm64
```

Build explicitly for Intel macOS:

```sh
export IDASDK=/Users/student/Documents/IDASDK
cmake -S . -B build-x86_64 -DCMAKE_OSX_ARCHITECTURES=x86_64
cmake --build build-x86_64
```

The produced plugin is a `.dylib` named `sigmaker.dylib` by default.

### Windows

```powershell
$env:IDASDK = 'C:\path\to\idasdk'
cmake -S . -B build
cmake --build build --config Release
```

The produced plugin is a `.dll` named `sigmaker64.dll` by default.

### Notes

- `IDASDK` should point to the SDK root that contains `include/ida.hpp`.
- If your SDK checkout keeps the actual SDK under `src`, pointing `IDASDK` at the parent folder is also supported.
- `IDASDK_ROOT` is still supported as an explicit CMake override.
- The CMake file looks for the standard IDA SDK library directories for each platform.
- `SetClipboardText()` uses `pbcopy` on macOS and the Win32 clipboard API on Windows.
