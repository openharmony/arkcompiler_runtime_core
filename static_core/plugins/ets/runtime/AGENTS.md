# AGENTS.md

This file provides guidance to AI when working with code in this repository.

## Project Metadata

- **name**: ETS Runtime plugin
- **purpose**: ETS language runtime implementation including async/await, ANI (Ark Native Interface), class linking, coroutines, and ETS-specific runtime features
- **primary language**: C++, Assembly

## About ETS Runtime

The **ETS runtime** extends the core ArkVM runtime with ETS language-specific features:
- **Async/Await**: Coroutine-based asynchronous programming
- **ANI**: Ark Native Interface for C/C++ interop
- **Class Linking**: ETS-specific class loading and resolution
- **Entrypoints**: Generated runtime intrinsics for ETS operations
- **Interop**: Integration with ArkTS-Dyn engines
- **Finalization**: Finalization registry for weak references

## Directory Structure

Main File Directories

```
plugins/ets/runtime/
├── CMakeLists.txt
│
├── ets_vm.cpp/h                # ETS VM implementation
├── ets_language_context.cpp/h    # ETS language context
├── ets_class_linker.cpp/h       # ETS class linker
├── ets_class_linker_extension.cpp/h
├── ets_class_linker_context.cpp/h # Class linker context
├── ets_class_root.cpp/h         # ETS class root
│
├── ets_coroutine.cpp/h          # Async/await support
│
├── ets_exceptions.cpp/h         # ETS exception handling
├── ets_call_stack.h             # ETS call stack tracking
├── ets_handle_scope.h           # GC handle scope
├── ets_handle.h                 # Object handles
│
├── ets_namespace_manager.cpp/h   # Namespace support
├── ets_annotation.cpp/h         # Annotation support
├── ets_object_state_table.h     # Object state tracking
├── ets_object_state_info.cpp/h
├── ets_mark_word.h              # GC marking
│
├── ets_entrypoints.cpp/h
├── ets_entrypoints.yaml         # Entrypoints definition
├── ets_compiler_intrinsics.yaml # Compiler intrinsics
├── ets_libbase_runtime.yaml     # Class/method mapping between ArkTS-Sta and Native
│
├── ets_itable_builder.cpp/h     # Interface table builder
├── ets_vtable_builder.h         # Virtual table builder
│
├── entrypoints/                 # Entrypoint assembly
│
├── asm_defines/                 # Assembly definitions
│
├── ani/                        # Ark Native Interface
│   ├── ani.h                    # Main ANI header
│   ├── ani_interaction_api.cpp/h # Interaction API
│   ├── ani_vm_api.cpp/h         # VM API
│   ├── ani_options.cpp/h        # ANI options
│   ├── ani_helpers.cpp/h         # Helper functions
│   ├── ani_converters.h         # Type converters
│   ├── ani_checkers.h           # Type checkers
│   ├── ani_type_info.h          # Type information
│   ├── ani_type_check.h         # Type checking
│   ├── ani_mangle.cpp/h         # Name mangling
│   ├── verify/                  # ANI verification
│   ├── arch/                   # Arch-specific ANI code
│   ├── docs/                   # ANI documentation
│   └── BUILD.gn
│
├── finalreg/                   # Finalization registry
│   ├── finalization_registry_manager.cpp/h
│   └── ...
│
├── ets_napi_env.h              # N-API interface
├── ets_native_library_provider.h
├── app_state.h                 # Application state
├── scoped_objects_fix.h        # Scoped objects
│
└── interop_js/                 # interop
```

## Key Components

### 1. ETS VM (`ets_vm.cpp/h`)

ETS (ArkTS-Sta) VM implementation extending core Runtime class.

### 2. Language Context (`ets_language_context.cpp/h`)

Implements `LanguageContext` interface for ETS

### 3. Class Linker (`ets_class_linker.cpp/h`)

ETS-specific class loading

### 4. ANI (`ani/`)

See `ani/docs/ani.md` for ANI documentation.

### 5. Finalization Registry (`finalreg/`)

Weak reference and finalization support for ETS objects.

## Build Commands

See @../../../AGENTS.md

## Code Style

See @../../../AGENTS.md

## Common Tasks

### Using ANI in Native Code

See `ani/docs/ani.md` for complete ANI guide.

## Testing

See @../../../AGENTS.md