# irtoc

`irtoc` is the Ruby-based IR-to-code pipeline used in `static_core` to define low-level compiler IR fragments and
lower them into runtime-linked object files. In the current tree it is used for:

- interpreter handlers
- fastpath entrypoints used from compiled code
- `NativePlus` entrypoints used from the interpreter side
- boundary and resolver stubs
- selected IR-builder and inline-generation helpers

The current implementation lives in `irtoc/lang/`, `irtoc/scripts/`, `irtoc/backend/`,
`runtime/CMakeLists.txt`, and `plugins/ets/runtime/CMakeLists.txt`.

## Current Pipeline

The current build pipeline is:

1. `.irt` sources are loaded by the Ruby frontend in `irtoc/lang/irtoc.rb`.
2. The frontend preprocesses helper syntax such as `:=` and plugin includes, then loads both real and pseudo
   instructions from `compiler/optimizer/ir/instructions.yaml`.
3. The frontend emits generated C++:
   - `irtoc_code.cpp`
   - `irtoc_code_llvm.cpp` when LLVM interpreter support is enabled
   - IR-builder or inline-generation C++ for the `ir-builder` and `ir-inline` APIs
4. The build creates a per-target executable such as `irtoc_fastpath_exec`.
5. That executable runs the compiler pipeline and produces:
   - `<target>.o`
   - optional `<target>_llvm.o`
   - `disasm.txt`
   - Release-only validation artifacts driven by `validation.yaml`

`runtime/CMakeLists.txt` wires the core `irtoc_interpreter` and `irtoc_fastpath` objects into the runtime. ETS adds
`irtoc_ets_fastpath` from `plugins/ets/runtime/CMakeLists.txt`.

## DSL Model

irtoc does not use a standalone parser. The frontend is Ruby code plus metaprogramming helpers from `irtoc/lang/`,
including:

- `function`
- `macro`
- `scoped_macro`
- `include_relative`
- preprocessing for `:=`

Instructions come from the compiler IR description, so irtoc is not a separate low-level ISA. It is a Ruby DSL over
current compiler IR semantics.

### Real and pseudo instructions

irtoc loads both normal IR instructions and pseudo instructions from `instructions.yaml`.

Current pseudo-instruction patterns used in the tree include:

- `Label`
- `Goto`
- `Else`
- `While`
- `LiveIn`
- `LiveOut`

### Data flow

`:=` is the current shorthand for creating or rebinding irtoc variables:

```ruby
sum := Add(lhs, rhs).i64
Return(sum).i64
```

It is also valid to build IR inline:

```ruby
Return(Add(lhs, rhs).i64).i64
```

### Control flow

Current irtoc code commonly uses explicit labels and jumps together with `If` / `Else`:

```ruby
function(:IncMax, params: {a: 'u64', b: 'u64'}, mode: [:Native]) {
    If(a, b).CC(:CC_GE) {
        bigger := Add(a, 1).u64
    } Else {
        bigger := Add(b, 1).u64
    }
    Return(bigger).u64
}
```

When control flow gets more explicit, labels are used directly:

```ruby
function(:SumToN, params: {n: 'word'}, mode: [:Native]) {
    sum_init := 0
    i_init := 0

    Label(:Loop)
    sum := Phi(sum_init, sum_next).word
    i := Phi(i_init, i_next).word
    sum_next := Add(sum, i).word
    i_next := AddI(i).Imm(1).word
    If(i_next, n).CC(:CC_LT) { Goto(:Loop) }

    Return(sum_next).word
}
```

Phi placement is still explicit. Do not rely on future auto-insertion behavior.

## Modes

Current mode names visible in the tree include:

- `Interpreter`
- `InterpreterEntry`
- `FastPath`
- `Boundary`
- `Native`
- `NativePlus`

`FastPathPlus` is a macro-mode. It expands into two generated variants:

- `FastPath`
- `NativePlus`

This is the current mechanism used when the same core implementation must exist both for compiled-code fastpaths and
for interpreter-side NativePlus entrypoints.

## Registers and types

Register maps are defined in `irtoc/scripts/common.irt`. The current tree keeps architecture-specific mappings for
`arm64`, `arm32`, and `x86_64`.

Common low-level types used across scripts include:

- `word` for pointer-sized integer values
- `ptr` for raw pointers
- `ref` for managed references
- `ref_uint` for the integer type matching the current reference size

Use `ref_uint` instead of hardcoding managed references to a fixed integer width.

Each function may also define register mappings and regalloc masks. The frontend converts those constraints into graph
setup such as `SetArchUsedRegs(...)`.

## Choosing irtoc

Use irtoc for short, hot, ABI-sensitive code paths where call overhead or precise low-level control matters more than
the optimizer flexibility of normal compiled code.

Good current fits are:

- interpreter handlers
- small fastpaths with a single explicit fallback
- boundary and resolver stubs
- tiny low-level helpers where managed code or C++ does not lower to the desired machine instructions

Avoid treating irtoc as a general replacement for ETS or C++ runtime code. If the implementation becomes large,
branch-heavy, or runtime-heavy, that is usually a sign to move the bulk of the logic back to managed code or C++ and
keep only the critical low-level fragment in irtoc.

## FastPath and NativePlus guidance

The common current pattern is:

1. do the smallest possible hot-path work in `FastPath`
2. jump or tail-call to an explicit slow path when checks fail
3. keep mode-sensitive ABI details in one place

Current scripts use both:

- `Intrinsic(:SLOW_PATH_ENTRY, ...)`
- `Intrinsic(:TAIL_CALL, ...)`

Do not document irtoc as globally unsupported on `arm32`. The toolchain still recognizes `arm32`, but many individual
fastpath and boundary entrypoints explicitly guard it out. Treat `arm32` support as per-function, not as a blanket
statement.

Also avoid absolute rules like "C++ calls are forbidden in the middle of fastpath". The current tree does have
`Call(...).Method(...)` usage. The safer rule is:

- avoid arbitrary runtime or GC-visible calls on hot `FastPath` code
- keep ABI and mode transitions explicit
- prefer a single slow-path tail call for fallback when possible

## Debugging and artifacts

The most useful generated artifacts are:

- `irtoc_code.cpp`
- `irtoc_code_llvm.cpp`
- `disasm.txt`
- `<target>.o`
- optional `<target>_llvm.o`
- `validation.yaml` in Release validation builds

Useful debugging workflow:

1. inspect generated C++ if you need to understand frontend lowering
2. inspect `disasm.txt` for the emitted code shape
3. use architecture-specific `objdump` on the generated object or final shared library when needed
4. set breakpoints by the generated entrypoint symbol name
5. correlate failures with runtime/compiler call sites and checked tests

Fastpath frames are often optimized around tail calls, so call stacks may not look like ordinary C++ call chains.

## Related Docs

- `../irtoc/AGENTS.md`
- `../docs/irtoc-intrinsics-for-interpreter.md`
- `../docs/runtime-compiled_code-interaction.md`
- `../docs/flaky_debugging.md`
