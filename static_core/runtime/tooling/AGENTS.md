# AGENTS

This file provides guidance for AI agents when working with code in the static core runtime tooling module.

## Project Overview

This is the **Static Core Runtime Tooling Module** of the Ark Compiler, part of the OpenHarmony ecosystem. It implements runtime debugging, profiling, and coverage analysis tools for **statically-typed ArkTS** (ArkTS-Sta) code running on the ARK virtual machine.

## Directory Structure

```
tooling/
├── inspector/                    # Runtime debugger (WebSocket server, breakpoints, evaluation)
├── sampler/                      # Sampling profiler (SIGPROF-based, ASPT format output)
├── coverage/                     # Coverage analysis (Python scripts, HTML reports)
├── evaluation/                   # Expression evaluation engine for debugger
├── backtrace/                    # Stack backtrace functionality
├── *.h, *.cpp                    # Core components: debugging, performance, interfaces, etc.
```

## Architecture

### Core Components

**Inspector System**
- WebSocket-based debugger using JSON-RPC protocol
- Breakpoint management: conditional breakpoints, URL-based breakpoints
- Expression evaluation engine, thread state debugging, and object inspection

**Sampler System**
- Dual-thread architecture: SamplerThread (signal delivery) + ListenerThread (file I/O)
- POSIX SIGPROF-based sampling profiler
- Outputs ASPT (Ark Sampling Profiler Trace) binary format

**Coverage System**
- Python-based code coverage analysis
- Generates HTML reports (full and incremental/diff-based)
- Supports host and device (HAP) modes

**Perf Counter System**
- Global `ark::tooling::g_perf` for creating scoped collectors
- Thread-safe performance measurement (CPU cycles, instructions, IPC)
- Requires build flag: `enable_perf_counters` (GN) or `-DENABLE_PERF_COUNTERS=true` (CMake)

**Debug Infrastructure**
- `debugger.h/cpp` - Core debugging interfaces and implementations
- `debug_inf.cpp` - Debug information processing and handling
- `pt_hooks_wrapper.h` - PtHooks interface wrapper for VM event callbacks
- `pt_default_lang_extension.h/cpp` - Language-specific debugging extensions
- `pt_thread_info.h` - Thread state information for debugging
- `pt_scoped_managed_code.h` - RAII wrapper for managed code scope management
- `thread_sampling_info.h` - Data structures for thread sampling
- `memory_allocation_dumper.h` - Memory allocation tracking and dumping

## Building

This module supports two build systems: **GN** (for OHOS full builds, integrated with the larger OpenHarmony/ArkCompiler build infrastructure) and **CMake** (for standalone builds).

### Build Commands

```bash
# Build static core runtime with tooling (OHOS full build)
./build.sh --product-name rk3568 --build-target libarkruntime
```

**Note**: The `build.sh` script is located at the OpenHarmony root, not in runtime_core.

### CMake Standalone Build

For building outside an OHOS local copy, use the CMake-based standalone build from the `static_core` root:

```bash
# Configure and build from static_core root
cmake -B out -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain/host_clang_default.cmake -GNinja .
cmake --build out --target arkruntime
```
