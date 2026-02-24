# ANI Module (Ark Native Interface)

## Introduction
The Ark Native Interface (ANI) is a native programming interface for the ArkTS runtime. It allows native C/C++ code to interact with ArkTS, providing capabilities to manage VM and environment instances, manage references, access classes, fields, and call methods. ANI serves a similar purpose to NAPI in Node.js, specifically tailored for the ArkTS environment.

## Directory Structure
Below is a brief overview of the files and directories in this module:

- `arch/`: Architecture-specific implementations (AMD64, ARM32, ARM64), providing entry points for both normal and verified modes.
- `docs/`: Detailed documentation regarding ANI specifications and features.
- `verify/`: Verification-specific logic. See [verify/AGENTS.md](./verify/AGENTS.md) for details.
- `ani_checkers.h`: Contains validation and check logic for ANI arguments and state.
- `ani_converters.h`: Utilities for converting between ANI types and internal runtime types.
- `ani_helpers.cpp/h`: Helper functions for common ANI implementation tasks.
- `ani_interaction_api.cpp/h`: Core implementation of the ANI environment functions (e.g., Object, Class, and Method operations).
- `ani_mangle.cpp/h`: Logic for name mangling and demangling to support ArkTS symbols.
- `ani_options_parser.cpp/h`: Parser for ANI initialization options.
- `ani_options.cpp/h`: Configuration and options management for ANI.
- `ani_type_check.h`: Type safety and checking mechanisms for ANI.
- `ani_type_info.h`: Runtime type information helpers.
- `ani_vm_api.cpp/h`: Implementation of VM lifecycle functions (e.g., `ANI_CreateVM`).
- `ani.h`: The public header file defining the ANI interface.
- `BUILD.gn`: Build configuration for the ANI module.
- `scoped_objects_fix.h`: RAII helpers for managing local references and code state.

## Verification
The ANI Verification Module provides a security layer by validating arguments and monitoring state. For more details on how to activate and develop for this module, see the [ANI Verification Agent Guide](./verify/AGENTS.md).

## Assembly Bridges and Entry Points
ANI uses architecture-specific assembly "bridges" to handle the transition between the ArkTS runtime and native C/C++ code. These are located in the `arch/` directory.

### Key Entry Points
- **Normal Mode**: Uses `EtsNapiEntryPoint`.
- **Verification Mode**: Uses `EtsNapiVerifyEntryPoint`.

### Implementation Hooks
The assembly code calls into C++ hooks defined in `ani_helpers.cpp` to manage the native state:
- **Begin Hooks**: `EtsNapiBegin` / `EtsNapiBeginVerify` (setup native frames, handle arguments).
- **End Hooks**: `EtsNapiEnd` / `EtsNapiEndVerify` (cleanup frames, handle return values).
- **Object End Hooks**: `EtsNapiObjEnd` / `EtsNapiObjEndVerify` (specialized for object returns).

## Fast Native Annotations
To further optimize performance, ANI supports special annotations that reduce the overhead of native calls by bypassing certain runtime state transitions or frame management.

- **`@ani.unsafe.Direct`**:
  - **Constraints**: No objects allowed in parameters or return values.
  - **Optimization**: No coroutine state switch (managed-to-native and vice versa). This is the fastest calling mode, bypassing almost all runtime overhead.
- **`@ani.unsafe.Quick`**:
  - **Capabilities**: Objects are allowed.
  - **Optimization**: Skips the coroutine state switch but still performs necessary frame management to handle object references safely.

These annotations are particularly useful for high-frequency, low-latency native operations where the full overhead of a standard ANI call is not justified.

## Testing Commands
To run tests for the ANI module, ensure you are in the project's build output directory (e.g., `static_core/out/`).

### Full Test Suite
To run all ANI-related tests:
```bash
ninja ani_tests
```

### Specific Test Case
To run a specific test (e.g., example calls):
```bash
ninja ani_test_example_calls_gtests
```

## Interface Synchronization
ANI is part of the OpenHarmony ecosystem. If you modify any public interfaces in `ani.h`, you **MUST** ensure the changes are synchronized with the interface repository:

- **Repository**: [interface_sdk_c](https://gitcode.com/openharmony/interface_sdk_c)
- **File**: `ani/ani.h`

## Typical Development Guidelines
- **Error Handling**: Always use `ANI_CHECK_RETURN_IF_EQ` or similar macros to validate inputs and check for pending exceptions.
- **Reference Management**: Use `ScopedManagedCodeFix` when entering native code from the runtime to ensure proper reference tracking.
- **Type Safety**: Leverage `ani_checkers.h` to verify that types match expected signatures before performing operations.
