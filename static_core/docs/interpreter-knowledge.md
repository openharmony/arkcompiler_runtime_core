# Interpreter Knowledge

This document covers only the error-prone boundaries in the interpreter execution pipeline. For irtoc code generation details see `irtoc.md` and `irtoc-intrinsics-for-interpreter.md`. For GC-interpreter interaction (safepoints, barriers) see `gc-knowledge.md` in the same directory.

## Interpreter Type Selection

`--interpreter-type` controls which interpreter is used, but the runtime may downgrade:

| Type | Entry point | Notes |
|------|-------------|-------|
| `llvm` | `ExecuteImplFast_LLVM` | Default (requires `PANDA_LLVM_INTERPRETER`), best performance |
| `irtoc` | `ExecuteImplFast` | Requires `PANDA_WITH_IRTOC`, irtoc-generated interpreter |
| `cpp` | `ExecuteImplInner<RuntimeInterface, ...>` | Hand-written C++ template interpreter, fallback |

### Downgrade Rules

The runtime automatically downgrades; do not assume `--interpreter-type` always takes effect:

| Condition | Downgrade to |
|-----------|-------------|
| `PANDA_LLVM_INTERPRETER` not defined | llvm → irtoc |
| `PANDA_WITH_IRTOC` not defined | irtoc → cpp |
| `ARK_USE_COMMON_RUNTIME` (CMC GC) | llvm/irtoc → cpp (when not explicitly set) |
| `USE_HWASAN` | any → cpp |
| `PANDA_SDK_LINUX_X64_FORCE_CPP_INTERPRETER` | any → cpp |
| `PANDA_TARGET_ARM32` | fixed cpp, irtoc unavailable |
| Dynamic language frame (`frame->IsDynamic()`) without explicit setting | defaults to cpp |
| `--debug-mode=true` | only cpp supported |

When debugging, **always specify** `--interpreter-type=cpp|irtoc|llvm` explicitly; do not rely on implicit defaults.

## Execution Entry

```
Execute()                     <- interpreter.cpp
    └── ExecuteImpl()         <- interpreter_impl.cpp
          ├── Select interpreter type (GetInterpreterTypeFromRuntimeOptions)
          └── ExecuteImplType()
                ├── LLVM:   ExecuteImplFast_LLVM / ExecuteImplFastEH_LLVM
                ├── IRTOC:  ExecuteImplFast / ExecuteImplFastEH
                └── CPP:    ExecuteImplInner<RuntimeInterface, IS_DYNAMIC, IS_PROFILE_ENABLED>
```

When `jumpToEh=true`, the exception-handling entry variant (EH suffix) is used.

## Frame Layout

Static language frames and dynamic language frames have different structures:

```
Static language frame (IS_DYNAMIC = false):
+-----------------------------+
|         ark::Frame          |
+-----------------------------+ <- payload
|         vregs[0]            |
|          ...                |
|         vregs[nregs-1]      |
+-----------------------------+ <- mirror
|         vregs[0]            |
|          ...                |
|         vregs[nregs-1]      |  <- mirror stores tags (for precise GC root identification)
+-----------------------------+

Dynamic language frame (IS_DYNAMIC = true):
+-----------------------------+
|         ark::Frame          |
+-----------------------------+ <- payload
|         vregs[0]            |
|          ...                |
|         vregs[nregs-1]      |  <- no mirror; TaggedValue carries type info inline
+-----------------------------+
```

### Do Not Mix Frame Operations

| Operation | Static frame | Dynamic frame |
|-----------|-------------|---------------|
| Get VReg | `StaticFrameHandler` | `DynamicFrameHandler` |
| tag/type info | mirror region | TaggedValue inline |
| Accumulator | `AccVRegisterT` (value + tag) | `AccVRegisterT` (value + tag) |

When modifying frame operation code, first confirm the frame's `IS_DYNAMIC` flag. `Frame::IS_DYNAMIC = 128` determines frame layout.

## Virtual Registers and Accumulator

- Virtual register size is 128 bits (64-bit value + 64-bit tag/padding), reducible to 64 bits via NaN-tagging
- The accumulator (`AccVRegisterT`) is implicitly addressed by some bytecodes and shared across frames
- Tags enable the GC to precisely distinguish references from primitives; do not remove tags for performance

## Dispatch

The interpreter uses indirect threaded dispatch (computed goto) to reduce dispatch overhead:

```
bytecode -> dispatch_table[opcode] -> handler -> next bytecode
```

- The `State` class holds the current PC, frame pointer, thread pointer, and dispatch table pointer
- When `PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES` is defined, these states are placed in hardware registers for speed
- The global register variable version accesses them via the `arch::regs::` namespace (`GetPc`, `GetFp`, `GetMirrorFp`, `GetThread`)

### Global Register Variable Caveats

- `arch/global_reg.h` **must never** be included in `interpreter.h` — it breaks ABI
- It is conditionally included in `state.h` (guarded by `PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES`)
- `SaveState()` / `RestoreState()` are used to save/restore global registers when calling into the runtime

## JIT/OSR Trigger Points

The interpreter detects hotness during execution and triggers compilation:

```
Interpreter execution loop
    |
    +-- Back-edge instruction (Jump backward)
    |     +-- Update hotness counter
    |           +-- Counter not overflowed -> continue interpreting
    |           +-- Counter overflowed + method already compiled -> trigger OSR
    |           +-- Counter overflowed + method not compiled -> submit JIT compilation task
    |
    +-- OSR entry (OsrEntry)
          +-- Look up OsrStateStamp -> frame conversion (IFrame -> CFrame)
```

- Hotness threshold is controlled by `--compiler-hotness-threshold`, default 3000
- OSR only triggers when `--compiler-enable-osr=true` and the frame has not been deoptimized (`!IsDeoptimized()`)
- Async methods (`HasAsyncAnnotation()`) do not support OSR
- OSR frame conversion is implemented in assembly (`runtime/bridge/`) because CFrame lives on the native stack

## Frame Flags

| Flag | Value | Meaning |
|------|-------|---------|
| `FORCE_POP` | 1 | Frame must be forcefully popped |
| `RETRY_INSTRUCTION` | 2 | Frame must retry the last instruction |
| `NOTIFY_POP` | 4 | Notify when frame is popped |
| `IS_DEOPTIMIZED` | 8 | Frame created by deoptimization; not eligible for OSR |
| `IS_STACKLESS` | 16 | Stackless frame (stackless interpreter mode) |
| `IS_INITOBJ` | 32 | initobj frame (stackless mode) |
| `IS_INVOKE` | 64 | Created within Method::Invoke |
| `IS_DYNAMIC` | 128 | Dynamic vs static frame layout selector |
| `IS_RESUMED` | 256 | Resumed from scheduler |

## irtoc Intrinsics and the Interpreter

irtoc-implemented intrinsics can serve both compiled code and the interpreter, but with different calling conventions:

| Mode | Calling convention | Used by |
|------|--------------------|---------|
| `FastPath` | Compiler ABI | Compiled code |
| `NativePlus` | Interpreter ABI (first arg is MethodPtr) | Interpreter |
| `FastPathPlus` | Generates both variants | Both |

### Limitations

- `SlowPath` intrinsics are not supported under `NativePlus` mode; use `Call` or `TailCall` instead
- Non-leaf functions cannot use `FastPathPlus` unchanged, because `FastPath` and `NativePlus` call sites differ
- When `NativePlus` calls another `NativePlus` function, a dummy MethodPtr first argument must be passed manually
- There is no built-in check for the current compilation mode; the developer must track the callee's mode manually

## Pre-Change Checklist

- Is the current frame static or dynamic? Do the operations match?
- Is the interpreter type selection correct? Has it been downgraded?
- Does the irtoc intrinsic calling convention match the current mode?
- Are OSR trigger conditions complete? Are deoptimized frames correctly excluded?
- Does global register variable save/restore cover all runtime call paths?
- Are GC barriers correctly inserted on reference writes?

## Code and Tests

Interpreter core code can be traced starting from `runtime/interpreter/`:

| Component | Anchor |
|-----------|--------|
| Execution entry | `runtime/interpreter/interpreter.cpp` -> `interpreter_impl.cpp` |
| Frame layout | `runtime/interpreter/frame.h` |
| Instruction handler base | `runtime/interpreter/instruction_handler_base.h` |
| Execution state | `runtime/interpreter/state.h` |
| Virtual registers | `runtime/interpreter/vregister.h`, `acc_vregister.h` |
| Runtime interface | `runtime/interpreter/runtime_interface.h` |
| Architecture-specific | `runtime/interpreter/arch/` |
| irtoc handler scripts | `irtoc/scripts/interpreter.irt`, `allocation.irt`, `gc.irt`, etc. |
| OSR entry | `runtime/osr.cpp`, `runtime/bridge/` |
| Deoptimization | `runtime/deoptimization.cpp`, `runtime/bridge/arch/` |

Frame and bridge tests use `stack_walker_test.cpp`, `c2i_bridge_test.cpp`, and `i2c_bridge_test.cpp` under `runtime/tests/`. irtoc interpreter validation uses `tests/irtoc-interpreter-tests/`.
