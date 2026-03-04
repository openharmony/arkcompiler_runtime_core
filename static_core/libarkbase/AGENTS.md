# AGENTS.md

This file provides guidance to AI when working with code in this repository.

## Project Metadata

- **name**: Libarkbase
- **purpose**: Foundation library providing OS abstraction, memory management, utilities, events, and task management for the Ark runtime
- **primary language**: C++

## About Libarkbase

**Libarkbase** is the foundation library that all other runtime components depend on. It provides:
- **OS Abstraction**: Cross-platform OS primitives (threads, files, memory, time)
- **Memory Management**: Arena allocators, memory pools, GC barriers
- **Utilities**: Logging, serialization, CPU features detection
- **Task Management**: Task queues and scheduling
- **Events**: Event system for runtime instrumentation

## Directory Structure

Main File Directories

```
libarkbase/
├── README.md                    # Documentation
│
├── os/                          # OS abstraction layer
│   ├── thread.cpp/h             # Thread wrapper
│   ├── mutex.cpp/h              # Mutex implementation
│   ├── file.cpp/h               # File operations
│   ├── filesystem.cpp/h          # Filesystem utilities
│   ├── time.cpp/h               # Time functions
│   ├── library_loader*.cpp      # Dynamic library loading
│   ├── system_environment.cpp/h  # Environment variables
│   ├── cpu_affinity.cpp/h       # CPU affinity control
│   ├── exec.cpp/h                # Process execution
│   ├── pipe.cpp/h                # Pipe operations
│   ├── kill.cpp/h                # Signal/kill handling
│   ├── property.cpp/h            # System properties
│   ├── mem.cpp/h                 # Memory operations
│   ├── mem_hooks.cpp/h           # Memory allocation hooks
│   ├── stacktrace.cpp/h          # Stack trace capture
│   ├── native_stack.cpp/h         # Native stack walking
│   ├── debug_info.cpp/h           # Debug information
│   ├── sighook.cpp/h              # Signal handling
│   ├── error.cpp/h                # Error handling
│   ├── failure_retry.h             # Retry utilities
│   ├── unique_fd.h                # Unique file descriptor wrapper
│   └── dfx_option.cpp/h           # Diagnostic options
│
├── mem/                         # Memory management
│   ├── mem.h                     # Memory management interface
│   ├── mem_config.cpp/h           # Memory configuration
│   ├── mem_pool.h/cpp            # Memory pool abstraction
│   ├── malloc_mem_pool*.h        # Malloc-based memory pool
│   ├── mmap_mem_pool*.h          # Mmap-based memory pool
│   ├── arena.h/cpp               # Arena allocator
│   ├── arena-inl.h               # Inline arena functions
│   ├── arena_allocator.h          # Arena allocator STL adapter
│   ├── arena_allocator.cpp         # Arena allocator implementation
│   ├── stack_like_allocator*.h    # Stack-like allocator
│   ├── code_allocator.cpp/h       # Code allocation (executable memory)
│   ├── pool_map.cpp/h            # Pool mapping
│   ├── pool_manager.cpp/h         # Pool management
│   ├── base_mem_stats.cpp/h       # Memory statistics
│   ├── alloc_tracker.cpp/h         # Allocation tracking
│   ├── gc_barrier.h               # GC barrier definitions
│   ├── mem_range.h                # Memory range utilities
│   ├── space.h                    # Memory space abstraction
│   ├── weighted_adaptive_tlab_average.h # TLAB sizing
│   ├── object_pointer.h            # Object pointer utilities
│   └── ringbuf/lock_free_ring_buffer.h # Lock-free ring buffer
│
├── arch/                        # Architecture-specific code
│
├── taskmanager/                 # Task management system
│
├── events/                      # Event system
│
├── serializer/                  # Serialization utilities
│
├── templates/                   # Code generation
│
├── globals.h                   # Global definitions
├── macros.h                    # Utility macros
├── concepts.h                  # concepts
├── cpu_features.h              # CPU features
├── test_utilities.h            # Test utilities
│
├── CMakeLists.txt              # CMake build
└── BUILD.gn                    # GN build
```

## Key Components

### OS Abstraction Layer

The `os/` directory provides cross-platform abstractions for:
- **Threading**: Thread creation, mutexes, condition variables
- **File I/O**: File operations, filesystem utilities
- **Memory**: Virtual memory, memory mapping
- **Time**: Timers, timestamps
- **Library Loading**: Dynamic library loading (dlopen/LoadLibrary)
- **Process Management**: Process execution, signals
- **Stack Traces**: Stack walking for debugging/profiling

### Memory Management

The `mem/` directory provides:
- **Arena Allocator**: Fast bump-pointer allocator for temporary allocations
- **Memory Pools**: Abstract memory pool interface with multiple implementations
- **Code Allocator**: Allocates executable memory for JIT/AOT code
- **Pool Manager**: Manages multiple memory pools
- **GC Barriers**: Definitions for write/read barriers used by GC

### Task Manager

Used by:
- JIT compiler task scheduling
- Parallel GC worker threads
- Other concurrent workloads

### Events System

Runtime instrumentation and tracing:
- Event definitions in `events.yaml`
- Generated event API for tracing runtime operations

## Build Commands

See @../AGENTS.md

## Code Style

See @../AGENTS.md

## Testing

See @../AGENTS.md
