# panda_guard — Agent Guide

This document is the **full working guide** for **`arkcompiler/runtime_core/panda_guard`**.

---

## What is panda_guard?

**panda_guard** is an **obfuscation tool** for Panda/Ark bytecode. It reads an input `.abc` file, builds a **pandasm::Program** in memory, applies configurable obfuscation (identifier renaming, property renaming, file name obfuscation, log removal, decorator obfuscation, compact mode, etc.), and writes an obfuscated `.abc` (and optionally a `.pa` file in debug mode).

Main capabilities:

- **ABC → Program**: Uses **abc2program::Abc2ProgramCompiler** to compile an `.abc` file into a **pandasm::Program**.
- **Obfuscation**: Renames records (source-file nodes), functions, classes, objects, properties, toplevel names, and file paths according to rules and reserved lists; can remove console log, apply decorator obfuscation, and compact output.
- **Name cache**: Can **apply** an existing name-cache file for stable re-obfuscation, and **print** a new name-cache file (JSON) mapping original names to obfuscated names for use by the frontend/source maps.
- **Output**: Emits obfuscated bytecode via **pandasm::AsmEmitter::Emit**; in debug mode can also dump the program to `.pa` using **PandasmProgramDumper**.

The **entry point** is the **panda_guard** executable (**guard_main.cpp**), which invokes **GuardDriver::Run**. The core logic lives in the **panda_guard_static** library.

---

## High-level pipeline and data flow

1. **Init** (**GuardDriver::Run**):
   - **GuardContext::Init(argc, argv)**: Parse CLI with **GuardArgsParser** (config file path, debug mode) → load **GuardOptions** from JSON → create **NameCache** and load apply-name-cache if configured → create **NameMapping** and add reserved names.
   - If **DisableObfuscation()** is true, return immediately.

2. **ABC → Program**:
   - **Abc2ProgramCompiler**: Open ABC, **CompileAbcFile()** → get **pandasm::Program**.
   - **GuardContext::CreateGraphContext(abc_file)** for graph-based analysis (e.g. cross-reference).

3. **Obfuscate** (**ProgramGuard::GuardProgram(program)**):
   - Build obfuscation model: **Program::Create()** — for each record in **program.record_table**, create a **Node** (or handle annotation); each Node builds **Function**, **Class**, **Object**, **Array**, **ModuleRecord**, **FilePath**, **UiDecorator**, **Annotation** entities from bytecode/instructions.
   - **Program::Obfuscate()** — for each entity that needs update, assign obfuscated names via **NameMapping** and update the program (record names, function names, literals, instructions, scope names, etc.). Optional steps: **RemoveConsoleLog**, **RemoveLineNumber**, **UpdateReference**.
   - Entities call **WriteNameCache** / **WriteFileCache** / **WritePropertyCache** to record mappings into **NameCache**.

4. **Write outputs**:
   - **NameCache::WriteCache()** — write name-cache JSON (if print-name-cache path is set).
   - If debug and obf `.pa` path set: dump program to `.pa`.
   - **pandasm::AsmEmitter::Emit** to the obfuscated `.abc` path.

5. **Finalize**: **GuardContext::Finalize()** (e.g. graph context cleanup).

Key data structures:

- **pandasm::Program**: In-memory program; modified in place by obfuscation.
- **Program** (guard): Holds **prog_** and **nodeTable_** (record name → **Node**). Each **Node** corresponds to a record (source file / JSON file / annotation) and contains **ModuleRecord**, **FilePath**, **functionTable_**, **classTable_**, **objectTable_**, **arrays_**, **uiDecorator_**, **annotations_**, etc.
- **Entity** hierarchy: **IEntity** (Create, Obfuscate), **Entity** (name_, obfName_, needUpdate_, Build, RefreshNeedUpdate, Update, WriteNameCache), **Node**, **Function**, **Class**, **Object**, **Property**, **Array**, **Annotation**, **UiDecorator**, etc.
- **GuardContext** (singleton): **GuardOptions**, **NameCache**, **NameMapping**, **GraphContext**.
- **NameCache**: Holds **ProjectNameCacheInfo** (fileCacheInfoMap, propertyCacheMap, fileNameCacheMap, etc.); Load(apply path), AddObf* methods, WriteCache().
- **NameMapping**: GetName / GetFileName / GetFilePath (with optional new generation), AddReservedNames, AddNameMapping; uses **NameGenerator** and **GuardOptions** for reserved/whitelist checks.

---

## Repository layout (where things live)

### Entry and driver

- **guard_main.cpp**: **main** → **GuardDriver::Run**.
- **guard_driver.h** / **guard_driver.cpp**: **GuardDriver::Run** — init context, open ABC, compile to program, create graph context, **ProgramGuard::GuardProgram**, optional dump `.pa`, **AsmEmitter::Emit** obf ABC, finalize.
- **guard4program.h** / **guard4program.cpp**: **ProgramGuard::GuardProgram** — constructs **Program** (obf), calls **Create()**, **Obfuscate()**, then **NameCache::WriteCache()**.

### Config and context

- **configs/guard_options.h** / **guard_options.cpp**: **GuardOptions** — load from JSON; **ObfuscationConfig** (paths, versions, **ObfuscationRules**). Getters for paths, flags (DisableObfuscation, IsExportObfEnabled, IsRemoveLogObfEnabled, IsDecoratorObfEnabled, IsCompactObfEnabled, IsPropertyObfEnabled, IsToplevelObfEnabled, IsFileNameObfEnabled), reserved names, record white list, keep paths, source name table, skipped remote HAR, normalized OHM URL, etc.
- **configs/guard_args_parser.h** / **guard_args_parser.cpp**: **GuardArgsParser** — parse CLI, return config file path and debug mode.
- **configs/guard_context.h** / **guard_context.cpp**: **GuardContext** singleton — **Init** (parser → options → name cache load → name mapping), **GetGuardOptions**, **GetNameCache**, **GetNameMapping**, **CreateGraphContext**, **GetGraphContext**, **IsDebugMode**, **Finalize**.
- **configs/guard_name_cache.h** / **guard_name_cache.cpp**: **NameCache** — **FileNameCacheInfo**, **ProjectNameCacheInfo**; Load(apply path), AddObf* APIs, WriteCache(), history/new cache handling.
- **configs/guard_graph_context.h** / **guard_graph_context.cpp**: **GraphContext** — built from panda file for analysis; CreateGraphContext, Finalize.
- **configs/version.h**: Version / build info.

### Name generation and mapping

- **generator/name_mapping.h** / **name_mapping.cpp**: **NameMapping** — GetName, GetFileName, GetFilePath; AddReservedNames, AddNameMapping, AddFileNameMapping; uses **NameGenerator** and **NameCache**.
- **generator/name_generator.h**: **NameGenerator** interface.
- **generator/order_name_generator.h** / **order_name_generator.cpp**: **OrderNameGenerator** implementation.

### Obfuscation model (obfuscate/)

- **obfuscate/program.h** / **program.cpp**: **Program** — Create (CreateNode per record, CreateAnnotation), Obfuscate, RemoveConsoleLog, RemoveLineNumber, UpdateReference; **nodeTable_**.
- **obfuscate/node.h** / **node.cpp**: **Node** — extends **Entity**; InitWithRecord, Build (scan instructions, CreateFunction/Class/Object/Array/ModuleRecord/FilePath/UiDecorator, etc.), Update, WriteNameCache; **moduleRecord_**, **filepath_**, **functionTable_**, **classTable_**, **objectTable_**, **arrays_**, **uiDecorator_**, **annotations_**, **pkgName_**, **sourceName_**, **obfSourceName_**, etc.
- **obfuscate/entity.h** / **entity.cpp**: **IEntity**, **Entity** (Create/Obfuscate, Build, RefreshNeedUpdate, Update, WriteNameCache, name_, obfName_, needUpdate_), **TopLevelOptionEntity**, **PropertyOptionEntity**.
- **obfuscate/function.h**, **class.h**, **method.h**, **object.h**, **property.h**, **array.h**, **module_record.h**, **file_path.h**, **ui_decorator.h**, **annotation.h**, **instruction_info.h**, **inst_obf.h**, **graph_analyzer.h** — entity types and instruction/graph helpers.

### Utilities

- **util/error.h** / **error.cpp**, **file_util.h** / **file_util.cpp**, **json_util.h** / **json_util.cpp**, **string_util.h** / **string_util.cpp**, **assert_util.h**.

### Tests

- **tests/BUILD.gn**: **PandaGuardUnitTest** (host_unittest_action) — sources in **unittest/** (guard_args_parser_test, guard_context_test, guard_name_cache_test, guard_options_test, name_generator_test, string_util_test), **util/test_util.cpp**; depends on **panda_guard_static**; **update_unittest_config** action for test JSON configs.
- **tests/unittest/configs/**: JSON configs for tests.
- **tests/script/update_test_config_json.py**: Script to update test config JSONs.

---

## Build and test

### Build

- **Library**: **panda_guard_static** — all obfuscation and config sources; deps: **abc2program_frontend_static**, **libarkassembler_frontend_static**, **libarkcompiler_frontend_static**, **libpandabase_frontend_static**, **libpandafile_frontend_static**.
- **Executable**: **panda_guard** — links **panda_guard_static**; built from **guard_main.cpp**.
- **Copy targets**: **panda_guard_build_ets** (Linux/sdk), **panda_guard_build_win_ets** (Windows), **panda_guard_build_mac_ets** (Mac) — copy the binary for the ETS build flow.

From ark root:

```bash
./ark.py x64.release  # or the target that builds panda_guard
```

### Test

- **PandaGuardUnitTest**: Host unit tests; run via the project test runner. Configs under **tests/unittest/configs/**; **PANDA_GUARD_UNIT_TEST_DIR** is set to the unittest directory for resolving config paths.

---

## How to add a new feature (checklist)

### A) New obfuscation rule or option

1. Add fields and getters to **GuardOptions** (and **ObfuscationConfig** / **ObfuscationRules** in **guard_options.h**). Extend **GuardOptions::Load** and JSON parsing in **guard_options.cpp** to read the new option.
2. Use the option in the appropriate layer: **Entity::RefreshNeedUpdate** (e.g. **PropertyOptionEntity**, **TopLevelOptionEntity**), **Node**, or **Program** (e.g. RemoveConsoleLog, RemoveLineNumber) so that only entities that need updating are obfuscated.
3. Add unit tests under **tests/unittest/** and, if needed, a new JSON config in **tests/unittest/configs/**.

### B) New entity type or instruction pattern

1. In **Node::Build** (or the relevant scanner), detect the new pattern via **InstructionInfo** / bytecode and create a new entity type (e.g. new class extending **Entity**), and add it to the Node's table (e.g. **functionTable_**, **classTable_**, or a new map).
2. Implement **Create**, **Build**, **RefreshNeedUpdate**, **Update**, and **WriteNameCache** / **WriteFileCache** / **WritePropertyCache** as needed. Use **NameMapping::GetName** (or GetFileName/GetFilePath) for stable renaming and reserved-name checks.
3. Ensure **Program::Create** / **Program::Obfuscate** and **UpdateReference** (or graph analyzer) account for the new entity so all references in the program are updated consistently.
4. Add tests (unit or small .abc/.pa scenarios) to cover the new pattern.

### C) Name cache format or apply/print behavior

1. Extend **ProjectNameCacheInfo** / **FileNameCacheInfo** (in **guard_name_cache.h**) if you need new cache sections or fields.
2. Update **NameCache::Load** (ParseAppliedNameCache, ParseHistoryNameCacheToUsedNames) and **NameCache::WriteCache** (BuildJson) and any **AddObf*** / getter APIs so apply and print stay in sync.
3. Update **NameMapping** (and **NameGenerator**) so that when applying a cache, existing mappings are used and new names are not reused from history. Add tests in **guard_name_cache_test** and **guard_context_test** with JSON configs that use apply/print name cache.

### D) New CLI argument or config key

1. Add the argument in **GuardArgsParser** and/or the config schema used by **GuardOptions::Load**.
2. Pass the value through **GuardContext** into **GuardOptions** or the component that needs it. Add or extend **guard_args_parser_test** / **guard_options_test** with the new option.

---

## Critical pitfalls / "do not do this"

- **Do not** modify the program (record_table, function_table, instructions) without going through the Entity/Node **Update** and **UpdateReference** path; otherwise references (e.g. in literals, scope names, bytecode) can become inconsistent.
- **Do not** generate new names for entities that are in the reserved list or record white list; **GuardOptions** and **NameMapping** (with AddReservedNames, AddNameMapping) must be used so reserved names and applied name cache are respected.
- **Do not** assume **needUpdate_** is always true; **RefreshNeedUpdate** depends on options (e.g. property/toplevel/file name enable flags and keep paths). Entities with **needUpdate_ == false** are skipped in **Obfuscate** but may still need to **WriteNameCache** when they are in the name-cache scope.
- **Do not** create **GuardContext** or **GuardOptions** manually in tests without initializing options and name cache as in **GuardContext::Init**; use the test configs and **PANDA_GUARD_UNIT_TEST_DIR** so paths and JSON configs resolve correctly.
- When adding new instruction patterns, **do not** forget to update **UpdateReference** and/or **GraphAnalyzer** so that cross-function or cross-record references (e.g. literals, scope names) are updated after renaming.

---

## Where to look (quick reference)

| Goal | Location |
|------|----------|
| Entry and pipeline | **guard_main.cpp**, **guard_driver.cpp**, **guard4program.cpp** |
| Config and context | **configs/guard_options.h**, **configs/guard_context.h**, **configs/guard_args_parser.h**, **configs/guard_name_cache.h** |
| Name generation | **generator/name_mapping.h**, **generator/order_name_generator.h** |
| Obfuscation model | **obfuscate/program.h**, **obfuscate/node.h**, **obfuscate/entity.h**, **obfuscate/function.h**, **obfuscate/class.h**, **obfuscate/method.h**, **obfuscate/property.h**, **obfuscate/object.h**, **obfuscate/module_record.h**, **obfuscate/instruction_info.h**, **obfuscate/inst_obf.h**, **obfuscate/graph_analyzer.h** |
| Utils | **util/error.h**, **util/json_util.h**, **util/assert_util.h** |
| Tests | **tests/unittest/*.cpp**, **tests/unittest/configs/*.json**, **tests/BUILD.gn** |
