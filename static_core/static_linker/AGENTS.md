# static_linker — Agent Guide

This document is the **full working guide** for **`arkcompiler/runtime_core/static_core/static_linker`**.

---

## What is static_linker?

**static_linker** is the Ark **static linker** for Panda (`.abc`) files. It reads one or more input `.abc` files, merges them into a single output file, resolves cross-file references (classes, methods, fields, foreign methods/fields), patches bytecode to use the merged indices, optionally strips unused code and debug info, and writes the result. It is used in the **static VM / ArkTS** toolchain (e.g. ahead-of-time linking of application and libraries).

Main capabilities:

- **Single entry**: **`Link(conf, output, input)`** — runs the full pipeline: Read → Merge → Parse → TryDelete → ComputeLayout → Patch → Write. Public API is in **linker.h** (namespace **ark::static_linker**).
- **Merge**: Combine multiple Panda files into one **ItemContainer**; resolve class redefinition (allowed only for *partial* classes such as `_GLOBAL`); merge regular classes, foreign classes, methods, fields, foreign methods/fields; deduplicate where applicable.
- **Parse**: Resolve annotations and references; build **knownItems_** (old → new) and **cameFrom_**; handle deferred annotation failures.
- **TryDelete**: Entry-point–based stripping of unused code when **strip-unused** is enabled (**entryNames** / **allFileIsEntry**).
- **Layout & patch**: Compute final layout; **CodePatcher** rewrites bytecode (string IDs, literal array IDs, indexed refs) to point into the merged container; line number program deduplication and debug info updates.
- **Config**: **stripDebugInfo**, **partial** / **remainsPartial** class sets, **entryNames**, **allFileIsEntry**. CLI options come from **options.yaml** (generated **link_options.h**).

Artifacts: **libarklinker** (shared/static library) and **ark_link** (executable). The executable supports `@file` list inputs and writes errors to stderr; **--show-stats** prints timing and item counts.

---

## High-level pipeline and data flow

1. **Input**: **Config** (stripDebugInfo, partial, remainsPartial, entryNames, allFileIsEntry) and a list of `.abc` paths. The **ark_link** binary may expand `@path` to a list of files (one path per line).

2. **Pipeline** (see **linker.cpp**):
   - **Read**: Open each input with **FileReader**, store in **readers_**; load into **ItemContainer** (conceptual; actual merge is in Merge).
   - **Merge**: Set entity offsets; **AddRegularClasses** (forward-declare regular classes, check redefinition); collect foreign classes into **knownItems_**; **FillRegularClasses**; iterate all indexable items and **MergeItem** (foreign methods/fields, etc.); run **deferredFailedAnnotations_**.
   - **Parse**: Resolve annotations and references; ensure all referenced entities exist in the merged container.
   - **TryDelete**: Remove unreachable code when strip-unused is enabled (entry points from **entryNames** or all classes if **allFileIsEntry**).
   - **ComputeLayout**: Finalize offsets/sizes of the merged container.
   - **Patch**: For each **CodeData**, **ProcessCodeData** builds **CodePatcher** changes (indexed, string, literal-array); **ApplyDeps** / **TryDeletePatch** / **Patch** apply them; line number and debug info are updated (e.g. **LinkerDebugInfoUpdater** in **linker_code_parser_context.cpp**).
   - **Write**: **cont_.Write(&writer)** to **output**.

3. **Output**: One linked `.abc` file; **Result** (errors, stats: elapsed times, itemsCount, classCount, codeCount, debugCount, deduplicatedForeigners).

Key data structures:

- **Context**: Holds **Config**, **Result**, **ItemContainer cont_**, **readers_**, **knownItems_** (old→new), **cameFrom_** (item→reader), **codeDatas_**, **CodePatcher patcher_**, **foreignFields_** / **foreignMethods_** maps, **literalArrayId_**.
- **CodePatcher**: **changes_** (IndexedChange, StringChange, LiteralArrayChange, string, or function); **ranges_**; **Add** / **ApplyDeps** / **TryDeletePatch** / **Patch** / **Devour**.
- **CodeData**: code vector, old/new **MethodItem**, **FileReader**, **patchLnp** flag.

---

## Repository layout (where things live)

### Entry and API

- **linker.h** — **Config**, **DefaultConfig()**, **Result**, **Link(conf, output, input)**. Namespace **ark::static_linker**.
- **linker.cpp** — **Link()** implementation: creates **Context**, runs Read → Merge → Parse → TryDelete → ComputeLayout → Patch → Write; fills **Result.stats.elapsed** and **Result.stats** (items, classes, code, debug, deduplicatedForeigners); **operator<<(Result::Stats)**.
- **link.cpp** — **ark_link** main: **PandArgParser** + **Options** (from **options.yaml**); **CollectAbcFiles** (expand `@file`); **ReadDependenciesFromConfig** (strip-unused, entry list); builds **Config** and calls **Link**; prints errors and optional stats.

### Context and patching

- **linker_context.h** — **Context**, **CodePatcher** (and change types), **CodeData**, **Helpers::BreakProto**; **ErrorDetail** / **ErrorToStringWrapper**; private merge/parse/patch helpers.
- **linker_context.cpp** — **Context::Read**, **Merge** (AddRegularClasses, FillRegularClasses, MergeItem), **Parse**, **TryDelete**, **ComputeLayout**, **Patch**; **MergeClass** / **MergeMethod** / **MergeField**; **MergeForeignMethod** / **MergeForeignField** (and Create); **TryFindField** / **TryFindMethod**; annotation/value mapping (**AnnotFromOld**, **ValueFromOld**, etc.); **ProcessCodeData** orchestration.
- **linker_code_parser_context.cpp** — **CodePatcher::Add** / **ApplyDeps** / **TryDeletePatch** / **Patch**; **LinkerDebugInfoUpdater** / **LinkerDebugInfoScrapper**; **Context::HandleStringId**, **Context::HandleLiteralArrayId**, **Context::ProcessCodeData** (bytecode walk, collect patcher changes).
- **linker_context_misc.cpp** — **ReprItem**, **ReprMethod**, **ReprValueItem** (for error/diagnostic output); **ErrorToStringWrapper::operator<<**; **DemangleName**; other helpers (used by tests via direct include of **linker_context_misc.cpp**).

### Build and options

- **CMakeLists.txt** — **panda_add_library(arklinker)** (linker.cpp, linker_context.cpp, linker_context_misc.cpp, linker_code_parser_context.cpp); **panda_add_executable(ark_link**, link.cpp); **panda_gen_options(ark_link**, options.yaml → link_options.h); test subdir.
- **BUILD.gn** — **arklinker** (shared), **arklinker_static**, **ark_link** executable; **ark_gen_file("link_options_h")**; **group("static_linker")**.
- **options.yaml** — **log-level**, **output**, **partial-classes**, **remains-partial-classes**, **show-stats**, **strip-debug-info**, **strip-unused**, **strip-unused-skiplist**. Namespace **ark::static_linker**; generates **generated/link_options.h** (e.g. **Options** class).

### Tests

- **tests/linker_test.cpp** — GTest: **TestSingle** (single .abc: build from .pa, link, optionally run with **ExecPanda**, compare to .gold); **TestMultiple** (multiple .abc with permutations, optional dedup stats); tests for hello_world, lit_array, exceptions, foreign method/field (and bad/overloaded/dedup variants), unresolved_global, lnp_dedup, strip-debug-info, **ReprValueItem** / **ReprArrayValueItem**, loop superclass (if **TEST_STATIC_LINKER_WITH_STS**).
- **tests/CMakeLists.txt** — **panda_add_gtest(static_linker_tests)**; **arklinker**, **arkruntime**, **arkassembler**; copy **data** to binary dir; list of .pa / .gold for gen.
- **tests/data/** — **single/** (e.g. hello_world.pa/.gold, lit_array, exceptions, lnp_dedup, unresolved_global); **multi/** (e.g. fmethod, ffield, dedup_field, dedup_method, bad_*, ffield_overloaded, fmethod_overloaded*, out.gold).

---

## Build and test

- **CMake**: From a Panda/Ark build tree that supports static_core: configure with **PANDA_WITH_TESTS**, build the **arklinker** and **ark_link** targets; run **static_linker_tests** (e.g. `ctest -R static_linker` or run the test binary).
- **GN**: Build **//path/to/static_linker:ark_link** and **:arklinker** (or **:static_linker** group). Tests may be in a separate test target if defined.
- **Options**: Generated header is **generated/link_options.h** (from **options.yaml**); **link.cpp** includes it as **generated/link_options.h** (path depends on build).

---

## How to add a new feature

1. **Config/CLI**: Add option in **options.yaml** (namespace **ark::static_linker**); rebuild to get **link_options.h**; in **link.cpp** map the option into **Config** (e.g. partial, remainsPartial, entryNames, stripDebugInfo) and pass **Config** to **Link**.
2. **Pipeline step**: If you need a new phase, add a **Context** method (e.g. **Context::NewStep()**) and call it from **Link()** in **linker.cpp** in the right order (e.g. after Merge, before Layout); keep error handling via **HasErrors()** / **GetResult()**.
3. **Merge rules**: For new item types or merge semantics, extend **MergeItem** in **linker_context.cpp** and the **knownItems_** / **cameFrom_** updates; for class-level rules use **AddRegularClasses** / **FillRegularClasses** and **CheckClassRedifinition**.
4. **Bytecode patching**: New instruction or reference types should be handled in **ProcessCodeData** / **HandleStringId** / **HandleLiteralArrayId** (or new handlers) in **linker_code_parser_context.cpp**; add **CodePatcher** change types in **linker_context.h** if needed and apply them in **CodePatcher::Patch** / **ApplyDeps** / **TryDeletePatch**.
5. **Tests**: Add single-file cases under **tests/data/single/** (**.pa** + **.gold** for stdout) or multi-file under **tests/data/multi/<scenario>/** (e.g. **1.pa**, **2.pa**, **out.gold**); add a **TEST(linkertests, ...)** in **linker_test.cpp** using **TestSingle** or **TestMultiple**; if you added **.pa**/**.gold** files, list them in **tests/CMakeLists.txt** (**test_asm_files** / copy) so they are present in the test run dir.

---

## Critical pitfalls / "do not do this"

- **Do not** change the order of pipeline stages (Read → Merge → Parse → TryDelete → ComputeLayout → Patch → Write) without understanding dependencies: e.g. Patch needs layout and knownItems_; TryDelete needs entry points and merged items.
- **Do not** assume input file order is fixed: multi-file tests permute order; merge and resolution must be order-independent where the spec allows.
- **Do not** add mutable global state for linker logic: **Context** holds all state; **Link()** is re-entrant modulo shared **Config** and input paths.
- **Do not** forget **remainsPartial**: Classes in **remainsPartial** (e.g. `_GLOBAL`) may keep unresolved methods/fields; otherwise unresolved refs are errors.
- **Do not** patch bytecode before **ComputeLayout**: indices and offsets are only final after layout.
- **Do not** use **ItemContainer** or **FileReader** after they have been moved or destroyed; **Context** keeps **readers_** and **cont_** for the whole pipeline.

---

## Where to look (quick reference)

| Goal | Location |
|------|----------|
| Main API and pipeline | **linker.h**, **linker.cpp** |
| Merge and context state | **linker_context.h**, **linker_context.cpp** |
| Bytecode parsing and patching | **linker_code_parser_context.cpp** |
| CodePatcher change application | **linker_context.h** (CodePatcher), **linker_code_parser_context.cpp** |
| Error formatting / Repr | **linker_context_misc.cpp**, **linker_context.h** (ErrorDetail) |
| CLI and options | **link.cpp**, **options.yaml**, generated **link_options.h** |
| Tests and test data | **tests/linker_test.cpp**, **tests/data/single/** and **tests/data/multi/** |
| Build | **CMakeLists.txt**, **BUILD.gn** |

---

## Dependencies

- **libarkfile**: **FileReader**, **ItemContainer**, **FileWriter**, **BytecodeInstruction**, file items (class, method, field, code, string, literal array, debug, etc.).
- **libarkbase**: **Logger**, **PandArgParser**, **macros**, **os/file**, **os/mutex**, **utils/utf**.
- **options**: Generated from **options.yaml** via **panda_gen_options** / **ark_gen_file** (templates/options).

Tests additionally depend on **arkassembler** (pandasm parser/emitter), **arkruntime** (for **ExecPanda**), and **arkstdlib**.
