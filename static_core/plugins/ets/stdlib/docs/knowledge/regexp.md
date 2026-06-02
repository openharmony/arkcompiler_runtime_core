# RegExp Knowledge

This document only covers architectural constraints and implementation boundaries of the RegExp module. For performance decision rules, see `performance.md`.

## Core Model

The ETS layer connects to the native layer via ANI (7 methods) and the intrinsic mechanism (1 method: parse), and the native layer wraps the PCRE2 third-party library.

```
ETS (RegExp.ets)                    Intrinsic (runtime/intrinsics/)        PCRE2
├ Constructor: parse() ─ Intrinsic → compile+validate (ets_libbase_runtime.yaml)
├ flags: buildCompileFlags()        Native (native/core/regexp/)           PCRE2
│        buildMatchFlags()          ├ ANI entry: Exec/Match/Test/Replace/Split  ├ pcre2_compile_8/16()
├ lastIndex: native/ETS co-managed  ├ Dispatch: EtsRegExp → RegExp8 / RegExp16 ├ pcre2_match_8/16()
└ Result: RegExpExecArray           ├ Shared: PrepareInputAndRun<>()           ├ pcre2_substitute_8/16()
   RegExpMatchArray                 │  ├ LATIN1_DIRECT  → RegExp8             └ Thread-local: compile_context
                                    │  ├ UTF16_DIRECT   → RegExp16               match_data (32 groups)
                                    │  └ LATIN1_TO_UTF16 → RegExp16            JIT: disabled / Unicode: enabled
```

### Result Type Hierarchy

```
Array<String | undefined> → RegExpResultArray (isCorrect, endIndex, groups)
                              ├─ RegExpMatchArray (match/matchAll: index?, input?)
                              └─ RegExpExecArray (exec/replace: index, input)
```

## File Locations

| Layer | Path | Responsibility |
| --- | --- | --- |
| ETS | `std/core/RegExp.ets` | RegExp class, flags computation, native method declarations |
| Intrinsic | `runtime/intrinsics/std_core_RegExp.cpp` | `StdCoreRegExpParse`: pattern validate + PCRE2 compile |
| Intrinsic Registration | `runtime/ets_libbase_runtime.yaml` | Intrinsic binding for `parse()` |
| ANI Registration | `native/core/regexp/RegExp.cpp` | `RegisterRegExpNativeMethods()` (7 methods, excluding parse) |
| Dispatch | `native/core/regexp/regexp_executor.{h,cpp}` | `EtsRegExp`: compile/execute dispatch |
| Execution | `native/core/regexp/regexp_8.{h,cpp}`, `regexp_16.{h,cpp}` | PCRE2 8-bit / 16-bit compile and match |
| Shared | `native/core/regexp/regexp_common.{h,cpp}` | Input dispatch, zero-copy string access |
| Specialized | `regexp_replace.{h,cpp}`, `regexp_split.{h,cpp}`, `regexp_matchall.{h,cpp}` | replace/split/matchAll |
| Auxiliary | `regexp_group_meta.h`, `regexp_exec_result.h` | Capture group metadata, result structs |

## PCRE2 Integration

| Configuration | Value |
| --- | --- |
| JIT | **Disabled** |
| Unicode | Enabled |
| Code unit width | 8-bit + 16-bit |
| Match limit | `10000000` |
| Thread-local cache | `compile_context` + `match_data` (32 groups) cached per TLS |

### Replace Dual Path

| Condition | Path | Native Method |
| --- | --- | --- |
| Replacement string has no `$` | PCRE2 native substitute | `replaceLiteralNativeImpl` → `pcre2_substitute` |
| Replacement string contains `$1`/`$<name>` | Return match array, ETS layer post-processes | `replaceNativeImpl` → ETS `String.getSubstitution()` |

### lastIndex Management

- global/sticky mode: **native layer** updates `lastIndex`
- `match()`/`matchAll()`: **ETS layer** manages

## Constraints

- **Do not** modify PCRE2 constant values in `RegExp.ets` — Reason: must remain consistent with PCRE2 library definitions, otherwise flags will be misaligned
- **Do not** allocate large buffers in the native layer — Reason: use thread-local pre-allocated match data to avoid frequent allocation
- `parse()` uses **intrinsic** binding (registered in `ets_libbase_runtime.yaml`), **do not** register it in `etsstdlib.cpp`/`RegExp.cpp` — Reason: parse is an intrinsic, not an ANI method; it follows the intrinsic registration path
- **Do not** enable PCRE2 JIT — Reason: it is disabled in the build configuration; enabling it will cause inconsistent behavior
- New native methods **must** synchronize ANI signature string updates — Reason: signature mismatch causes runtime crashes
- New flags **must** update both ETS `buildCompileFlags()`/`buildMatchFlags()` and native `SetFlags()` — Reason: flag inconsistency causes incorrect regex behavior

## Pre-Modification Checklist

- Has the ANI signature string been updated?
- Has the new flag been handled in both ETS and native?
- Do both RegExp8 and RegExp16 need to be modified?
- Thread safety: are you depending on non-thread-local PCRE2 resources?

## Code and Tests

- ETS source: `std/core/RegExp.ets`
- Native source: `native/core/regexp/`
- PCRE2 build: `../../../cmake/third_party/pcre2/CMakeLists.txt`
- Test directory: `../tests/ets_func_tests/std/core/regexp/`
