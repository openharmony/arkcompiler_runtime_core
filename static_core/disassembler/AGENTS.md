# disassembler (static_core) — Agent Guide

This document is the **full working guide** for **`arkcompiler/runtime_core/static_core/disassembler`**.

---

## What is the disassembler (static_core)?

The **disassembler (static_core)** converts **Panda/Ark binary bytecode** (`.abc` files) into **pandasm assembly text** for the **static VM / ArkTS** toolchain. It builds an in-memory **pandasm::Program** and can serialize it to a stream (LITERALS, RECORDS, METHODS, STRING sections), with optional **ProgInfo** for debug/line/column info. It also provides **DisasmBackedDebugInfoExtractor**, which implements **panda_file::DebugInfoExtractor** using disassembly-derived source and line/column/local variable tables.

This is a **different** implementation from **`arkcompiler/runtime_core/disassembler`** (the dynamic / runtime_core disassembler). The two use different namespaces (`ark::disasm` vs `panda::disasm`), different build targets (`arktsdisassembler`, `arkts_disasm` / output `ark_disasm_static` vs `arkdisassembler`, `ark_disasm`), and different dependencies (libarktsfile, libarktsassembler, runtime profiling vs libpandafile, libarkassembler). The **static_core** version adds **DisasmBackedDebugInfoExtractor** (debug info backed by disassembly) and **profiling** integration; it does not generate **get_ins_info.cpp**.

Main capabilities:

- **Binary → Program**: Open a `.abc` (by path, or via **SetFile** with an existing **panda_file::File**), then **Disassemble()** to fill **prog_** (record_table, function_table, literalarray_table, module arrays, string table). Overloads: **Disassemble(filenameIn, quiet, skipStrings)**, **Disassemble(const File&, ...)**, **Disassemble(unique_ptr<File>&, ...)**.
- **Profiling**: **SetProfile(fname)**; destructor calls **profiling::DestroyProfile(profile_, fileLanguage_)**. Used for static runtime profiling when disassembling.
- **Debug info extractor**: **DisasmBackedDebugInfoExtractor** extends **panda_file::DebugInfoExtractor** and provides **GetLineNumberTable**, **GetColumnNumberTable**, **GetLocalVariableTable**, **GetParameterInfo**, **GetSourceFile**, **GetSourceCode**, **IsUserFile** / **SetUserFile**, backed by disassembly-derived data (e.g. **Disassembly** with sourceCode, lineTable, parameterInfo, localVariableTable).
- **Serialization**: **Serialize(os, addSeparators, printInformation)** for the full program; **Serialize(method, os, printInformation, lineTable)** for a single method. **CollectInfo()** fills **progInfo_** for verbose output.
- **Plugin metadata**: **GetLanguageSpecificMetadata()** (generated from plugin YAMLs) for language-specific annotations.

Public API is the **`ark::disasm::Disassembler`** class in **disassembler.h**. The **`arkts_disasm`** executable (output name **ark_disasm_static**) is a CLI that parses options (help, verbose, quiet, skip-strings, with_separators, debug, debug-file, version, input_file, output_file) and calls **Disassemble**, optionally **CollectInfo**, and **Serialize**. Built as **arktsdisassembler** (shared) and **arktsdisassembler_frontend_static** (static).

---

## High-level pipeline and data flow

1. **Input**: **Disassemble()** — either open file by path or use **SetFile()** to provide a **panda_file::File**. Options: **quiet**, **skipStrings**.

2. **DisassembleImpl** (internal):
   - Clear and fill: **record_name_to_id_**, **method_name_to_id_**, **string_offset_to_name_**, **modulearray_table_**, **prog_** (Program).
   - **GetRecords()**: Iterate classes, build **prog_.record_table** and **prog_.function_table** (GetRecord, GetMethods, GetFields, GetMethod, etc.).
   - **GetLiteralArrays()**: Fill **prog_.literalarray_table** and **modulearray_table_**.
   - **GetLanguageSpecificMetadata()**: Generated; plugin hooks.

3. **CollectInfo** (optional): Build **DebugInfoExtractor** (or use disasm-backed extractor); fill **progInfo_** (RecordInfo, MethodInfo, line/column/local variable tables) for **Serialize(..., printInformation = true)**.

4. **Serialize**: Write source binary comment → LITERALS → RECORDS → METHODS → STRING. Uses **prog_**; if **printInformation** is true, uses **progInfo_** for extra per-instruction/line/column info.

5. **DisasmBackedDebugInfoExtractor**: Construct with **panda_file::File** and optional **onDisasmSourceName** callback. Caches disassembly per method and implements **DebugInfoExtractor** by returning line/column/local variable/parameter/source file/source code from that cache. **IsUserFile** / **SetUserFile** control user-file filtering.

Key data structures:

- **Disassembler**: **file_** (or external file), **prog_** (Program), **progInfo_** (ProgInfo), **progAnn_** (ProgAnnotations); **record_name_to_id_**, **method_name_to_id_**, **string_offset_to_name_**, **modulearray_table_**; **profile_**, **fileLanguage_** for profiling.
- **DisasmBackedDebugInfoExtractor**: Extends **DebugInfoExtractor**; holds per-method **Disassembly** (sourceCode, lineTable, parameterInfo, localVariableTable); **sourceNamePrefix_**, **onDisasmSourceName_**.
- **accumulators.h**: **ProgInfo**, **RecordInfo**, **MethodInfo**, **ProgAnnotations**, **RecordAnnotations**, **LabelTable**, **IdList**.

---

## Repository layout (where things live)

### Entry and main logic

- **disasm.cpp**: **arkts_disasm** main — **Options** (help, verbose, quiet, skipStrings, with_separators, debug, debug-file, version, input_file, output_file), **PandArgParser**, then **Disassembler::Disassemble**, optional **CollectInfo**, **Serialize** to file. Uses **ark::** logger and **libarkfile** version.
- **disassembler.h** / **disassembler.cpp**: **ark::disasm::Disassembler** — **Disassemble** (overloads), **Serialize** (full program and single method), **CollectInfo**, **SetProfile**, **SetFile**, **GetMethod**, **GetProgInfo**; private **DisassembleImpl**, **GetRecord**, **AddMethodToTables**, **GetLiteralArray**/ **GetLiteralArrayByOffset**, **FillLiteralData**/ **FillLiteralArrayData**, literal parsing, **BytecodeInstructionToPandasmInstruction** (generated), serialization helpers. Includes **libarkfile** accessors, **runtime/profiling/profiling-disasm-inl.h**, **accumulators.h**, **disasm_plugins.inc**.

### Debug info extractor (static_core only)

- **disasm_backed_debug_info_extractor.h** / **disasm_backed_debug_info_extractor.cpp**: **DisasmBackedDebugInfoExtractor** — extends **panda_file::DebugInfoExtractor**; constructor (file, onDisasmSourceName); **GetLineNumberTable**, **GetColumnNumberTable**, **GetLocalVariableTable**, **GetParameterInfo**, **GetSourceFile**, **GetSourceCode**, **IsUserFile**, **SetUserFile**; internal **Disassembly** struct and **GetDisassembly** / **GetDisassemblySourceName**.

### Supporting and generated

- **accumulators.h**: **ProgInfo**, **RecordInfo**, **MethodInfo**, **ProgAnnotations**, **RecordAnnotations**, **LabelTable**, **IdList**.
- Generated under **$target_gen_dir**: **bc_ins_to_pandasm_ins.cpp**, **opcode_translator.cpp** (from **templates/** via **isa_gen_ark_disam**); **type_to_pandasm_type.cpp** (from **$ark_root/disassembler/templates/type_to_pandasm_type.cpp.erb** + libarkfile types.yaml); **disasm_plugins.inc**, **get_language_specific_metadata.inc** (from **templates/** + **ark_plugin_options_yaml**). No **get_ins_info.cpp** in static_core build.

### Plugin integration

- **plugin_arkdisassembler_sources** — Extra sources from plugins (e.g. ETS disassembler).
- **disasm_plugins.inc.erb**, **get_language_specific_metadata.inc.erb** — Plugin-driven; data from **ark_plugin_options_yaml**.

### Tests

- **tests/**: **extractor_test.cpp**, **functions_test.cpp**, **instructions_test.cpp**, **labels_test.cpp**, **literals_test.cpp**, **metadata_test.cpp**, **records_test.cpp**, **test_debug_info.cpp**. Run via the project test runner.

---

## Build and test

### Build

- **Libraries**: **arktsdisassembler** (shared), **arktsdisassembler_frontend_static** (static). Sources: **arkdisassembler_sources** (bc_ins_to_pandasm_ins, opcode_translator, type_to_pandasm_type, **disasm_backed_debug_info_extractor**, disassembler) + **plugin_arkdisassembler_sources**. Deps: **disasm_plugins_inc**, **get_language_specific_metadata_inc**, **isa_gen_ark_disam_*** (bc_ins_to_pandasm_ins, opcode_translator), **type_to_pandasm_type_cpp**, **libarktsassembler**, **libarktsbase**, **libarktsfile**, **runtime:arkruntime_header_deps**. Config: **arkdisassembler_public_config** (include_dirs: assembler, assembler gen, disassembler gen, disasm_plugins_inc output dir).
- **Executable**: **arkts_disasm** (output_name **ark_disasm_static**) — links **arktsdisassembler_frontend_static**; **install_enable = false**.

From ark root (with static VM / disassembler enabled as needed):

```bash
./ark.py x64.release  # or the target that builds arktsdisassembler / arkts_disasm
```

### Test

- Unit tests under **static_core/disassembler/tests/**; run via the project test runner. **extractor_test** exercises **DisasmBackedDebugInfoExtractor**.

---

## How to add a new feature (checklist)

### A) New serialization or metadata

1. Implement or extend the relevant **Serialize*** or **Get*Info** / **GetMetaData** methods in **disassembler.cpp**. If you need new fields on **ProgInfo**/ **RecordInfo**/ **MethodInfo**, add them in **accumulators.h** and populate them in the appropriate CollectInfo/GetRecordInfo/GetMethodInfo path.
2. Keep **disassembler.h** in sync (new public helpers if needed). Add or extend tests under **tests/**.

### B) New bytecode instruction or opcode mapping

1. Instruction decoding and pandasm emission are **generated** from ISA (assembler + libarkfile). Update the ISA so **templates/bc_ins_to_pandasm_ins.cpp.erb** and **templates/opcode_translator.cpp.erb** regenerate correctly. Do not hand-edit **bc_ins_to_pandasm_ins.cpp** or **opcode_translator.cpp**.
2. Rebuild and run **instructions_test** and any instruction-related tests.

### C) DisasmBackedDebugInfoExtractor behavior

1. Extend **DisasmBackedDebugInfoExtractor** (or the per-method **Disassembly** cache) to compute or store the new debug info. Implement the corresponding **DebugInfoExtractor** override (**GetLineNumberTable**, **GetColumnNumberTable**, **GetLocalVariableTable**, **GetParameterInfo**, **GetSourceFile**, **GetSourceCode**, **IsUserFile**).
2. Ensure the disassembler fills the data that the extractor needs (e.g. when **CollectInfo** is used or when a callback provides source names). Add or extend **extractor_test**.

### D) Plugin or language-specific metadata

1. Plugins register via **ark_plugin_options_yaml**. Implement the plugin's disassembler part and list sources in the plugin's config so they are in **plugin_arkdisassembler_sources**. Extend **disasm_plugins.inc.erb** or **get_language_specific_metadata.inc.erb** so your plugin's metadata hook is generated and called from **GetLanguageSpecificMetadata()**.

---

## Critical pitfalls / "do not do this"

- **Do not** mix code or headers from **runtime_core/disassembler** (panda::disasm, get_ins_info, PANDA_WITH_ECMASCRIPT) with **static_core/disassembler** (ark::disasm, DisasmBackedDebugInfoExtractor, profiling). They are separate implementations with different namespaces and build targets.
- **Do not** edit generated files (**bc_ins_to_pandasm_ins.cpp**, **opcode_translator.cpp**, **type_to_pandasm_type.cpp**, **disasm_plugins.inc**, **get_language_specific_metadata.inc**) by hand. Change the `.erb` templates and/or plugin YAMLs and regenerate.
- **Do not** assume **ProgInfo** is populated before **CollectInfo()** is called; Serialize with **printInformation == true** only makes sense after CollectInfo.
- **Do not** open or serialize without ensuring the disassembler has a valid file (via **Disassemble(path)** or **SetFile**); the CLI and callers should handle open failure.
- When adding **DisasmBackedDebugInfoExtractor** behavior, ensure the disassembler actually fills the data the extractor reads (e.g. line table, source code) in the code paths that the extractor uses.

---

## Where to look (quick reference)

| Goal | Location |
|------|----------|
| Entry and API | **disasm.cpp**, **disassembler.h**, **disassembler.cpp** |
| Debug info (static_core) | **disasm_backed_debug_info_extractor.h**, **disasm_backed_debug_info_extractor.cpp** |
| Accumulators | **accumulators.h** |
| Generated | **templates/bc_ins_to_pandasm_ins.cpp.erb**, **templates/opcode_translator.cpp.erb**, **templates/disasm_plugins.inc.erb**, **templates/get_language_specific_metadata.inc.erb**; **$ark_root/disassembler/templates/type_to_pandasm_type.cpp.erb** |
| Build | **BUILD.gn** (arktsdisassembler, arktsdisassembler_frontend_static, arkts_disasm, isa_gen_ark_disam, type_to_pandasm_type_cpp, disasm_plugins_inc, get_language_specific_metadata_inc) |
| Tests | **tests/extractor_test.cpp**, **tests/functions_test.cpp**, **tests/instructions_test.cpp**, **tests/labels_test.cpp**, **tests/literals_test.cpp**, **tests/metadata_test.cpp**, **tests/records_test.cpp**, **tests/test_debug_info.cpp** |
