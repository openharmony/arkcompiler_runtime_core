# Static ABC Verifier

Lightweight structural integrity verifier for Static ABC (ArkTS) bytecode files,
designed for cloud-side deployment in application marketplace review pipelines.

## Architecture

```
┌─────────────────────────────────────────────────────┐
│        Java verifier wrapper (Maven JAR + JNI)       │
│           com.oh.ark:static-abc-verifier            │
│                                                     │
│  StaticAbcVerifier.verify(filePath)                 │
│              │                                      │
│              ├── NativeLoader (extracts .so from JAR)│
│              ▼                                      │
│  ┌─────────── JNI ──────────┐                       │
│  │ nativeVerifyFile()       │                       │
│  └──────────────────────────┘                       │
└──────────────┬──────────────────────────────────────┘
               │
┌──────────────▼──────────────────────────────────────┐
│          C++ Core (libstaticabcverifier.so)          │
│                                                     │
│  ┌────────────┐  ┌──────────────────────┐           │
│  │  AbcFile    │  │  StaticAbcVerifier   │           │
│  │  (parser)   │  │  - VerifyMagic()     │           │
│  │             │  │  - VerifyVersion()   │           │
│  │  Open()     │  │  - VerifyChecksum()  │           │
│  │  GetHeader()│  │  - VerifyFileSize()  │           │
│  │  GetClasses │  │  - VerifyClassIndex()│           │
│  └────────────┘  │  - VerifyRegionHdrs() │           │
│                  │  - ...                │           │
│                  └──────────────────────┘           │
│                                                     │
│  C API: StaticAbcVerifyFile(path, errorBuf, len)  │
└─────────────────────────────────────────────────────┘
```

## Directory Structure

```
static_abc_verifier/
├── CMakeLists.txt              # Standalone CMake build
├── build.sh                    # Build automation script
├── README.md
├── src/
│   ├── abc_file.h / .cpp       # ABC binary format parser
│   ├── verifier.h / .cpp       # Verification logic
│   ├── verify_api.h / .cpp     # C API (for JNI and external consumers)
│   └── main.cpp                # CLI tool entry point
├── jni/
│   └── static_abc_verifier_jni.cpp   # JNI bridge
├── tests/
│   ├── CMakeLists.txt          # GTest-based test build
│   ├── test_helper.h           # Test utilities (ABC builders)
│   ├── abc_file_test.cpp       # AbcFile unit tests
│   ├── verifier_test.cpp       # Verifier unit tests
│   ├── verify_api_test.cpp     # C API tests
│   └── standalone_test.cpp     # No-dependency standalone test runner
└── java-verifier-wrapper/      # Java/JNI binding + Maven packaging for native core
    ├── pom.xml                 # Maven project
    └── src/
        ├── main/java/com/oh/ark/verifier/
        │   ├── StaticAbcVerifier.java   # Main API
        │   ├── VerifyResult.java        # Result object
        │   └── NativeLoader.java        # Native lib loader
        ├── main/resources/native/
        │   ├── linux-x86_64/            # x86_64 .so files
        │   └── linux-aarch64/           # aarch64 .so files
        └── test/java/com/oh/ark/verifier/
            ├── StaticAbcVerifierTest.java  # JUnit tests
            └── StandaloneTest.java        # No-dependency tests
```

## Building

### Prerequisites

- CMake >= 3.14
- GCC/G++ with C++17 support
- JDK 8+ (for JNI and Java wrapper build)
- Maven 3.x (for packaging the Java wrapper JAR, optional)
- aarch64-linux-gnu-g++ (for aarch64 cross-compilation)

### Quick Start

```bash
# Build native libraries (current platform)
./build.sh native

# Run C++ tests
./build.sh test

# Build JNI library and publish Java wrapper JAR to local Maven repo
./build.sh publish-local

# Clean all artifacts
./build.sh clean
```

### Manual Build

```bash
# C++ native build
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DBUILD_JNI=OFF
cmake --build build/native --parallel $(nproc)

# C++ tests (standalone, no GTest required)
g++ -std=c++17 -I src -o build/standalone_test tests/standalone_test.cpp \
    src/abc_file.cpp src/verifier.cpp src/verify_api.cpp
./build/standalone_test

# JNI library
JAVA_HOME=/path/to/jdk cmake -S . -B build/jni -DBUILD_JNI=ON -DBUILD_TESTS=OFF
cmake --build build/jni --target static_abc_verifier_jni

# Java wrapper (manual compile without Maven)
javac -d java-verifier-wrapper/target/classes java-verifier-wrapper/src/main/java/com/oh/ark/verifier/*.java
cp build/jni/libstaticabcverifier_jni.so java-verifier-wrapper/src/main/resources/native/linux-x86_64/
```

### Cross-compilation for aarch64

```bash
cmake -S . -B build/aarch64-jni \
    -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
    -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
    -DBUILD_JNI=ON -DBUILD_TESTS=OFF

cmake --build build/aarch64-jni --target static_abc_verifier_jni
cp build/aarch64-jni/libstaticabcverifier_jni.so \
   java-verifier-wrapper/src/main/resources/native/linux-aarch64/
```

## Usage

### CLI Tool

```bash
./static_abc_verifier /path/to/file.abc
# PASS: /path/to/file.abc is a valid static ABC file.
# or
# FAIL: /path/to/file.abc verification failed:
#   [E2] Invalid magic: expected 'PANDA', got bytes [...]
```

### Java wrapper (Maven JAR)

Add dependency (after local Maven publish):

```xml
<dependency>
    <groupId>com.oh.ark</groupId>
    <artifactId>static-abc-verifier</artifactId>
    <version>1.0.0-SNAPSHOT</version>
</dependency>
```

```java
import com.oh.ark.verifier.StaticAbcVerifier;
import com.oh.ark.verifier.VerifyResult;

// Verify from file path (persist uploads to a temp .abc file first if needed)
VerifyResult result = StaticAbcVerifier.verify("/path/to/file.abc");
if (result.isValid()) {
    System.out.println("Valid ABC file");
} else {
    System.err.println("Error: " + result.getErrorMessage());
}
```

### C API

```c
#include "verify_api.h"

char errorBuf[4096];
int ret = StaticAbcVerifyFile("/path/to/file.abc", errorBuf, sizeof(errorBuf));
if (ret == 0) {
    printf("Valid\n");
} else {
    printf("Invalid: %s\n", errorBuf);
}
```

## Verification Checks

Error codes match `ErrorCode` in `verifier.h` (C API / JNI return the first error’s numeric code).  
`E0` means success; `E1`–`E19` correspond to enum values `1`–`19`.

### Phase 1 — Basic layout (always on)

Parses the ABC header and index tables with `AbcFile` only (no `libarkfile`).

| Check | Code | Description |
| --- | --- | --- |
| File open / parse | E1 (`FILE_OPEN_FAILED`) | Cannot open path, or buffer too small / invalid for `AbcFile` |
| Magic | E2 (`INVALID_MAGIC`) | First 8 bytes must be `PANDA` + three `\0` |
| Version | E3 (`INVALID_VERSION`) | Version quad in `[0.0.0.6, 0.0.0.7]` (`STATIC_MIN_VERSION` … `STATIC_MAX_VERSION` in `abc_file.h`) |
| Checksum | E4 (`CHECKSUM_MISMATCH`) | Adler-32 over file bytes `[12 .. fileSize)` matches header `checksum` |
| Declared file size | E5 (`FILE_SIZE_MISMATCH`) | Header `fileSize` equals actual mapping length |
| Header fields | E6 (`INVALID_HEADER`) | `fileSize` ≥ header struct size; non-zero `classIdxOff`, `literalarrayIdxOff`, `indexSectionOff`, `lnpIdxOff` within `[0, fileSize)` |
| Class index | E7 (`INVALID_CLASS_INDEX`) | Class index table in range; each class entity offset `< fileSize` |
| Literal array index | E8 (`INVALID_LITERAL_ARRAY_INDEX`) | Literal index table in range; each literal array offset `< fileSize` |
| Region headers | E9 (`INVALID_REGION_HEADER`) | Each region `start ≤ end`; class/method/field/proto sub-index regions within file |
| Foreign section | E10 (`INVALID_FOREIGN_SECTION`) | If `foreignSize > 0`, `[foreignOff, foreignOff + foreignSize)` within file |

### Phase 2 — Deep structure (`USE_ARKFILE=ON` CMake option)

Uses `libarkfile` to walk classes, methods, code, and literal payloads. Runs **only if Phase 1 produced no errors** (same `Verify()` call).

| Check | Code | Description |
| --- | --- | --- |
| Source language | E11 (`INVALID_SOURCE_LANG`) | Class `SourceLang` field, when present, must be an allowed static-ABC language value |
| Class records | E12 (`INVALID_CLASS_DATA`) | Non-external classes: superclass id offset (if non-zero) in range; field name pool offsets in range; or class blob fails to parse |
| Method pool | E13 (`INVALID_METHOD_DATA`) | Non-external methods: method name and prototype offsets `< fileSize` |
| Code blob | E14 (`INVALID_CODE_DATA`) | Code entity offset in range; `CodeDataAccessor` parse failure; or code parse error in method pipeline |
| Literal arrays | E15 (`INVALID_LITERAL_ARRAY_DATA`) | Per-array span has `numItems`; items use known literal tags; values not truncated; reference-type slots point within file; parse failures |
| Bytecode opcodes | E16 (`INVALID_BYTECODE`) | Each instruction’s primary opcode is valid for the ISA |
| Virtual registers | E17 (`INVALID_REGISTER_INDEX`) | Operand register indices `< numVregs + numArgs` |
| Jump targets | E18 (`INVALID_JUMP_TARGET`) | For jump-flag instructions, computed target PC in `[0, codeSize)` |
| Try / catch | E19 (`INVALID_TRY_CATCH`) | Try `startPc` / `length` and catch `handlerPc` within code size |

External classes and external methods are skipped where `libarkfile` cannot expand accessors. Empty class index / index section / literal-array section short-circuit deep passes to avoid invalid index walks.

## Design Decisions

1. **Two-tier build**: Phase 1 stays self-contained (single `AbcFile` parser, easy to ship). Phase 2 links pre-built `libarkfile` / `libarkbase` (see `USE_ARKFILE` in `CMakeLists.txt`) for deeper structural checks without pulling in the full Ark runtime verifier.

2. **No VM / ClassLinker**: Unlike `static_core/verification`, this tool only inspects file layout and bytecode structure, not full semantic typing.

3. **Extensible pipeline**: Checks are methods on `StaticAbcVerifier` wired from `Verify()`. Multiple errors are collected where applicable; Phase 2 is skipped if Phase 1 already failed.
