# ArkTS Concurrency - Static Core

## 缩写说明

| 缩写 | 全称 | 说明 |
| :--- | :--- | :--- |
| **ArkTS-Dyn** | ArkTS-Dynamic | 动态模式下的 ArkTS |
| **ArkTS-Sta** | ArkTS-Static | 静态模式下的 ArkTS |

This directory contains the concurrency subsystem of **ArkTS-Sta** (Ark TypeScript Static Core), part of the ArkCompiler project. ArkTS has two versions:
- **ArkTS-Dyn**: Dynamic version (not covered in this directory)
- **ArkTS-Sta**: Static version (covered here - static_core)

The concurrency subsystem provides a comprehensive framework for asynchronous and parallel execution, ranging from low-level coroutine runtime to high-level language APIs.

## Architecture Overview

```
User-Facing Layer
    │
    ▼
Standard Library Layer (../../plugins/ets/stdlib)
    │
    ▼
Runtime Layer (../../plugins/ets/runtime)
    │
    ▼
Core Coroutine Runtime (./)
    │
    ▼
Foundation Layer (../fibers)
```

## Directory Structure

```
./                                         # Core coroutine runtime (C++)
./coroutine.h/cpp                          # Base coroutine class
./coroutine_manager.h/cpp                  # Manager interface and implementation
./stackful_coroutine*                      # Fiber-based implementation
./threaded_coroutine*                      # Thread-based implementation
./coroutine_events.h                       # Synchronization events
./priority_queue.h                         # Scheduling queue
./native_stack_allocator/                  # Stack allocation for fibers

../fibers/                                 # User-mode context switching (C++ + Assembly)
../fibers/fiber_context.h/cpp              # Context switching API
../fibers/arch/                            # Architecture-specific implementations
../fibers/arch/aarch64/                    # ARM64 support
../fibers/arch/amd64/                      # x86_64 support
../fibers/arch/arm/                        # ARM32 support

../../plugins/ets/runtime/                 # ETS runtime (C++)
../../plugins/ets/runtime/ets_coroutine.h/cpp          # ETS coroutine implementation
../../plugins/ets/runtime/types/                       # Runtime types (Job, Promise, SyncPrimitives, Atomics)
../../plugins/ets/runtime/intrinsics/                  # Standard library implementations
../../plugins/ets/runtime/interop_js/                  # JavaScript interoperability
../../plugins/ets/runtime/libani_helpers/              # ANI helpers

../../plugins/ets/stdlib/                  # Standard library (ArkTS/TypeScript)
../../plugins/ets/stdlib/std/core/          # Core constructs (Coroutine, Job, Promise, SyncPrimitives)
../../plugins/ets/stdlib/std/concurrency/   # High-level APIs (launch, Atomics, AsyncLock, taskpool)
../../plugins/ets/stdlib/std/containers/    # Concurrent collections (ConcurrentHashMap, BlockingQueue)
../../plugins/ets/stdlib/escompat/          # JavaScript ES compatibility

../../docs/coroutines/                     # Runtime-level technical documentation
../../plugins/ets/doc/concurrency/         # Language-level user documentation
../../plugins/ets/tests/                    # Test suites
```

## Module Descriptions

### `./` - Core Coroutine Runtime
Low-level coroutine execution engine in C++. Provides two implementation strategies:
- **Stackful coroutines**: Fiber-based with user-level context switching for efficiency
- **Threaded coroutines**: OS thread-based implementation for simplicity

Key components:
- Base coroutine lifecycle: CREATE → RUNNABLE → RUNNING → BLOCKED → TERMINATING
- Worker scheduling with priority-based queue (LOW, MEDIUM, HIGH, CRITICAL)
- Event-based synchronization (CompletionEvent, GenericEvent, TimerEvent)
- Worker groups and affinity masks for load balancing

### `../fibers` - Foundation Layer
User-mode context switching library providing fiber primitives. Cross-platform support for ARM32, ARM64, and AMD64 architectures. Zero system call design for maximum efficiency. Used by stackful coroutines as the foundation for context switching.

### `../../plugins/ets/runtime` - ETS Runtime
ETS (Ark TypeScript) runtime implementation. Extends base coroutine functionality with ETS-specific features including synchronization primitives (Mutex, Event, RWLock, CondVar), atomic operations (AtomicFlag, AtomicInt), Promise and Job implementations, and JavaScript interoperability via NAPI.

### `../../plugins/ets/stdlib` - Standard Library
User-facing APIs for ArkTS applications. Written in ArkTS (TypeScript variant). Provides:
- High-level concurrency APIs (launch, taskpool, WorkerGroup)
- Synchronization primitives (AsyncLock, Mutex, Event, RWLock)
- ES2023-compliant Atomics namespace
- Concurrent collections (ConcurrentHashMap, BlockingQueue variants)
- Promise-based async/await
- JavaScript ES compatibility layer

### Documentation
- **Runtime docs** (../../docs/coroutines): Technical implementation details for runtime engineers
- **Language docs** (../../plugins/ets/doc/concurrency): User-facing API specifications for developers

### Test Suites (../../plugins/ets/tests)
Comprehensive validation including atomic operations tests, coroutine tests, container tests, taskpool tests, JavaScript interoperability tests, and performance benchmarks.

## Execution Model

ArkTS uses a **single-threaded coroutine model**:
- **Jobs**: Units of work executed by coroutines
- **Workers**: OS threads that host multiple coroutines
- **Suspension Points**: Coroutines suspend at await/lock points without blocking workers
- **Event Loop**: Workers process coroutines from priority queues
- **Synchronization**: Event-based blocking with wait/notify mechanisms

## Technology Stack

- **Languages**: C++ (runtime), Assembly (context switching), ArkTS (stdlib)
- **Key Technologies**: Fiber-based coroutines, Panda VM, NAPI
- **Platform Support**: ARM32, ARM64, AMD64 (x86_64)
- **Build System**: CMake

## Related Documentation

- Runtime implementation details: ../../docs/coroutines/coroutines.md
- Language API specification: ../../plugins/ets/doc/concurrency/*.rst
