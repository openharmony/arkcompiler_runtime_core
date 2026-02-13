# AGENTS.md

This file provides guidance to AI when working with code in this repository.

## Project Metadata

- **name**: Platforms
- **purpose**: Platforms implementations for libarkbase OS abstractions
- **primary language**: C++, Assembly

## About Platforms

The **platforms/** directory contains platform-specific implementations that abstract OS differences:
- **unix/** - Unix-like systems (Linux, macOS, BSD)
- **windows/** - Windows platform
- **ohos/** - sample OpenHarmony runtime code
- **common/** - Common code shared across platforms

## Directory Structure

Main File Directories

```
platforms/
├── CMakeLists.txt                    # Main CMake configuration
│
├── unix/                             # Unix-like platforms (Linux, macOS)
│   └── libarkbase/
│       ├── thread.cpp/h              # Thread implementation (pthread)
│       ├── mutex.cpp/h              # Mutex (pthread_mutex)
│       ├── file.cpp/h               # File operations (POSIX)
│       ├── filesystem.cpp/h          # Filesystem utilities
│       ├── library_loader_load.cpp   # dlopen implementation
│       ├── library_loader_resolve_symbol.cpp # dlsym
│       ├── exec.cpp/h               # Process execution (fork/exec)
│       ├── pipe.cpp/h               # Pipe creation
│       ├── kill.cpp/h               # Signal/kill handling
│       ├── system_environment.cpp    # Environment variables
│       ├── cpu_affinity.cpp/h       # CPU affinity (sched_setaffinity)
│       ├── property.cpp/h            # System properties
│       ├── mem.cpp/h                # Memory operations (mmap, mprotect)
│       ├── mem_hooks.cpp/h          # Memory allocation hooks
│       ├── sighook.cpp/h            # Signal handling
│       ├── signal.h                  # Signal definitions
│       ├── native_stack.cpp/h         # Native stack walking
│       ├── error.cpp/h               # Error handling (errno)
│       ├── unique_fd.h              # Unique file descriptor (auto-close)
│       ├── trace.cpp/h               # Tracing utilities
│       └── futex/
│           ├── mutex.cpp/h           # Futex-based mutex
│           └── fmutex.cpp/h         # Futex implementation
│
├── windows/                          # Windows platform
│   └── libarkbase/
│       ├── thread.cpp/h              # Thread implementation (Win32 threads)
│       ├── mutex.cpp/h              # Mutex (CRITICAL_SECTION)
│       ├── file.cpp/h               # File operations (Win32 API)
│       ├── filesystem.cpp           # Filesystem utilities
│       ├── library_loader.cpp       # LoadLibrary implementation
│       ├── system_environment.cpp   # Environment variables
│       ├── cpu_affinity.cpp/h       # CPU affinity (SetThreadAffinity)
│       ├── pipe.cpp                 # Pipe (CreatePipe)
│       ├── kill.cpp                 # Process termination (TerminateProcess)
│       ├── mem.cpp/h               # Memory operations (VirtualAlloc)
│       ├── mem_hooks.cpp/h         # Memory hooks
│       ├── trace.cpp                # Tracing
│       ├── time.h                   # Time functions
│       ├── error.cpp                # Error handling (GetLastError)
│       ├── unique_fd.h              # Handle wrapper
│       └── windows_mem.h            # Windows-specific memory
│
├── ohos/                            # Sample OpenHarmony platform
│
├── common/                           # Common platform code
│
└── target_defaults/                  # Target default configuration
```

## Build Commands

See @../AGENTS.md

## Code Style

See @../AGENTS.md

## Testing

See @../AGENTS.md