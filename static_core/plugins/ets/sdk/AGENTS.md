# AGENTS.md

This file provides guidance to the AI agent when working with code in this repository.

## Project Overview

This is the **ETS SDK** for the **ArkCompiler runtime**, providing HarmonyOS/OpenHarmony standard APIs (@ohos.* namespace). Unlike the stdlib in `../stdlib/` which provides core language features, this SDK focuses on utility APIs for ArkTS applications.

### Architecture

The SDK is organized into two main directories:

- **`api/`** - HarmonyOS standard API implementations using the `@ohos.*` namespace
  - `@ohos.util.*` - Collection classes (ArrayList, HashMap, LinkedList, Stack, Queue, etc.)
  - `@ohos.buffer.ets` - Buffer/array buffer utilities
  - `@ohos.uri.ets`, `@ohos.url.ets` - URI/URL parsing and handling
  - `@ohos.xml.ets` - XML pull parser
  - `@ohos.base.ets` - Base types (BusinessError)
  - `UtilHelper.ets` - Native helper functions

- **`arkts/`** - ArkTS-specific extensions
  - `@arkts.math.Decimal.ets` - Decimal arithmetic
  - `@arkts.collections.ets` - BitVector and other collections

- **`native/`** - C++ implementations using ANI (Ark Native Interface)
  - `main.cpp` - Entry point for native bindings
  - `api/` - Native implementations for TextDecoder, TextEncoder, StringDecoder, XmlPullParser, Util

### Module System

- SDK files declare `package api;` or use `export` without package declaration
- Module naming uses `@ohos.` prefix for HarmonyOS APIs
- Import pattern: `import { BusinessError } from "@ohos.base";`
- The `arktsconfig.json` defines path mappings for the TypeScript compiler
- Files use the `export` keyword to expose APIs

## Build System

The SDK uses **CMake** for building. The build process:

1. **Compile each `.ets` file to `.abc` bytecode** using `compile_ets_code()`
2. **Link all `.abc` files** into a single `etssdk.abc` using `ark_link`
3. **Build native library** (`ets_sdk_native.so`) for native method implementations

### Key Build Targets

```bash
# From the runtime_core root (../../../)
cmake -B build -DCMAKE_BUILD_TYPE=Release -GNinja
cmake --build build --target etssdk       # Build complete SDK
cmake --build build --target ets_sdk_native  # Build native library only
```

### Build Output

- `.abc` files: `${CMAKE_CURRENT_BINARY_DIR}/*.abc`
- Linked SDK: `${CMAKE_CURRENT_BINARY_DIR}/etssdk.abc`
- Native library: `${PANDA_BINARY_ROOT}/lib/ets_sdk_native.so`

## Code Style Guidelines

The SDK follows similar style guidelines to the stdlib (see `../stdlib/AGENTS.md`), with key differences:

### Package Declaration
- Files in `api/` use: `package api;` followed by `export namespace util { ... }`
- Files in `arkts/` use: `export namespace collections { ... }`
- Some files (like `@ohos.base.ets`) use direct `export` without package

### Native Method Declarations

Native methods are declared with the `native` keyword and implemented in C++:

```typescript
export class UtilHelper {
    static {
        loadLibrary("ets_sdk_native")  // Loads the native library
    }

    public static native generateRandomUUID(entropyCache: boolean): string
}
```

The corresponding C++ implementation uses ANI signatures:
```cpp
extern "C" ani_status *util*UtilHelper_generateRandomUUID(...)
```

### Error Handling

SDK uses `BusinessError` from `@ohos.base` for API errors:
```typescript
import { BusinessError } from "@ohos.base";

const TypeErrorCodeId: int = 401;
const OutOfBoundsErrorCodeId: int = 10200001;

function createBusinessError(code: int, message: string) {
    let err = new BusinessError();
    err.code = code;
    err.name = 'BusinessError';
    err.message = message;
    return err;
}
```

## Native Integration

### ANI (Ark Native Interface)

The native layer provides C++ implementations for:
- **TextEncoder/TextDecoder** - Character encoding conversions (UTF-8, UTF-16LE, etc.)
- **StringDecoder** - String decoding utilities
- **XmlPullParser** - XML parsing
- **Util** - UUID generation, error string retrieval

### Native Bindings Pattern

1. ETS declares a static block with `loadLibrary("ets_sdk_native")`
2. Native methods marked with `native` keyword
3. C++ files in `native/api/` implement the functions with ANI signatures
4. The bindings are registered via `extern "C"` exports

### Adding Native Methods

To add a new native method:
1. Add method declaration with `native` keyword in `.ets` file
2. Implement in C++ in `native/api/*.cpp`
3. Update `native/CMakeLists.txt` to include new source file
4. Use ANI functions (`env->Object_GetFieldByName_*`, `env->CallMethod_*`) for ETS interop

## File Organization in CMakeLists.txt

The `SDK_SRCS` variable in `CMakeLists.txt` explicitly lists all source files to be compiled:
```cmake
set(SDK_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.buffer.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.util.ArrayList.ets"
    # ... etc
)
```

**Important**: When adding new `.ets` files, add them to `SDK_SRCS` list. The build uses a foreach loop to compile each file and link the resulting `.abc` files.

## Special Conventions

### Collection Classes

Collection classes (ArrayList, HashMap, etc.) follow this pattern:
- Implement `Iterable<T>` and optionally `JsonReplacer`
- Use `$iterator()` for custom iterator implementation
- Use `$_get()` and `$_set()` for indexed access
- Provide `convertToArray()`, `clone()`, `clear()`, `isEmpty()` methods
- Validate indices with private helper methods (`checkIndex`, `checkRange`)

### Constants

Error code constants use descriptive names with `Id` suffix:
```typescript
const TypeErrorCodeId: int = 401;
const OutOfBoundsErrorCodeId: int = 10200001;
```

### JSON Serialization

Classes implementing `JsonReplacer` must provide:
```typescript
public jsonReplacer(): Record<String, Any> {
    // Return object representation for JSON.stringify
}
```

## Dependencies

- **etsstdlib** - Standard library (must be built first)
- **ark_link** - Bytecode linker
- **arkruntime** - C++ runtime library for native code

## Verification

When not cross-compiling, the build runs verifier on the generated SDK:
```bash
${RUN_VERIFIER_DST_DIR}/run_verifier.sh '${verifier_bin}' '${ETS_STD_LIB}' '${ETS_SDK_ABC}'
```

## File Headers

All ETS files must include the Apache 2.0 license header (see `../stdlib/AGENTS.md` for the full header format).
