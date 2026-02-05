# disassembler (runtime_core) — Agent Guide

This document is the **full working guide** for **`arkcompiler/runtime_core/disassembler`** (the **dynamic / runtime_core** disassembler).

There is a **separate** disassembler under **`arkcompiler/runtime_core/static_core/disassembler`** (static / ArkTS). That one uses namespace **ark::disasm**, different build targets (**arktsdisassembler**, **arkts_disasm**), adds **DisasmBackedDebugInfoExtractor** and profiling support, and uses libarktsfile/libarktsassembler. Do not confuse the two.

---

## What is the disassembler (runtime_core)?

The **disassembler** converts **Panda/Ark binary bytecode** (`.abc` files) into **pandasm assembly text** — a human-readable representation that can be inspected, diffed, or fed back into the assembler.

Main capabilities:

- **Binary → Program**: Opens a `.abc` file and builds an in-memory **pandasm::Program** (records, functions, literal arrays, module literal arrays, string table).
- **Metadata and debug info**: Optionally collects **ProgInfo** (record/method metadata, line/column tables, local variable tables) via **DebugInfoExtractor**.
- **Serialization**: Writes the program to an output stream in pandasm text format (LITERALS, RECORDS, METHODS, STRING sections with optional separators and optional "informative" method/instruction info).

Public API is the **panda::disasm::Disassembler** class in **disassembler.h**. The **ark_disasm** executable is a thin CLI that calls **Disassemble**, optionally **CollectInfo**, and **Serialize**.

---

## High-level pipeline and data flow

1. **Input**: Path to a panda file (`.abc`). Options: **quiet** (enables all skip-* behavior), **skip_strings** (emit string IDs instead of literal strings to reduce output size).

2. **Disassemble** (main entry):
   - Open file with **panda_file::File::Open**.
   - Clear and fill: **record_name_to_id_**, **method_name_to_id_**, **string_offset_to_name_**, **modulearray_table_**, **prog_** (Program).
   - **GetRecords()**: Iterate classes, build **prog_.record_table** and **prog_.function_table** (via GetRecord / GetMethods / GetFields / GetMethod, etc.).
   - **GetLiteralArrays()**: Fill **prog_.literalarray_table** and **modulearray_table_**.
   - **GetLanguageSpecificMetadata()**: Generated; calls into plugin-provided metadata hooks (e.g. ECMAScript/ArkTS-specific annotations).

3. **CollectInfo** (optional, used when "verbose"):
   - Builds **DebugInfoExtractor** over the opened file.
   - For each record/method: **GetRecordInfo** / **GetMethodInfo** / **GetInsInfo** to fill **prog_info_** (RecordInfo, MethodInfo, line/column/local variable tables). Used when serializing with **print_information == true**.

4. **Serialize**:
   - Writes: source binary comment → LITERALS (literalarray_table, module arrays) → RECORDS → METHODS → STRING.
   - Record/function serialization uses **prog_**; if **print_information** is true, uses **prog_info_** for extra per-instruction/line/column info.

Key data structures:

- **pandasm::Program prog_**: The main in-memory program (record_table, function_table, literalarray_table).
- **ProgInfo prog_info_**: Optional detailed info per record/method (debug tables, instruction info); populated only after **CollectInfo()**.
- **ProgAnnotations prog_ann_**: Method and record annotations collected during disassembly.
- **Maps**: **record_name_to_id_**, **method_name_to_id_**, **string_offset_to_name_**, **modulearray_table_** for name↔entity resolution and module literals.

---

## Repository layout (where things live)

### Entry and main logic

- **disassembler.h**  
  - Public API: **Disassemble()**, **CollectInfo()**, **Serialize()**.  
  - Helpers: **GetRecord**, **GetMethod**, **AddMethodToTables**, **GetLiteralArray**, **BytecodeInstructionToPandasmInstruction**, annotation/string/module helpers, **GetProgInfo()**, **GetColumnNumber**/**GetLineNumber**.  
  - Includes **accumulators.h** (ProgInfo, RecordInfo, MethodInfo, LabelTable, IdList, ProgAnnotations) and generated **disasm_plugins.inc** (plugin hooks).
- **disassembler.cpp**  
  - Implements all Disassembler methods: GetRecords, GetLiteralArrays, GetRecord/GetMethod/GetFields, instruction decoding, try/catch mapping, metadata and annotations, serialization of literals/records/methods/strings.  
  - Includes generated **get_language_specific_metadata.inc**.
- **disasm.cpp**  
  - **ark_disasm** main: argument parsing (input_file, output_file, verbose, quiet, skip_strings, debug, version), then **Disassemble** → optional **CollectInfo** → **Serialize** to file.

### Supporting headers

- **accumulators.h**  
  - **MethodInfo**, **RecordInfo**, **ProgInfo**, **ProgAnnotations**, **AnnotationList**, **RecordAnnotations**, **LabelTable**, **IdList**.

### Generated code (do not edit by hand)

Generated under **$target_gen_dir** from BUILD.gn/CMake:

- **bc_ins_to_pandasm_ins.cpp** (from **templates/bc_ins_to_pandasm_ins.cpp.erb**)  
  - Implements **BytecodeInstructionToPandasmInstruction**; driven by ISA (assembler + libpandafile ISAPI).
- **get_ins_info.cpp** (from **templates/get_ins_info.cpp.erb**)  
  - Implements **GetInsInfo** (per-instruction debug/metadata info).
- **opcode_translator.cpp** (from **templates/opcode_translator.cpp.erb**)  
  - Bytecode opcode → pandasm opcode mapping.
- **type_to_pandasm_type.cpp**  
  - From **libpandafile/types.yaml**; maps Panda file types to pandasm types.
- **get_language_specific_metadata.inc** (from **templates/get_language_specific_metadata.inc.erb**)  
  - Implements **GetLanguageSpecificMetadata()**; calls plugin-specific getters (from **plugin_options.yaml** / concat_plugins_yamls).
- **disasm_plugins.inc** (from **templates/disasm_plugins.inc.erb**)  
  - Injected into **disassembler.h** (e.g. plugin-specific members or includes).

### Templates

- **templates/**  
  - **bc_ins_to_pandasm_ins.cpp.erb**, **get_ins_info.cpp.erb**, **opcode_translator.cpp.erb** — ISA-based codegen.  
  - **get_language_specific_metadata.inc.erb**, **disasm_plugins.inc.erb** — plugin-based.  
  - **type_to_pandasm_type.cpp.erb** — type mapping (data from libpandafile).

### Tests

- **tests/**  
  - Unit tests: e.g. **disassembler_imm_tests.cpp**, **disassembler_line_number_test.cpp**, **disassembler_column_number_test.cpp**, **disassembler_string_test.cpp**, **disassembler_module_literal_test.cpp**, **disassembler_user_annotations_test.cpp**, **disassembler_system_annotations_test.cpp**, **disassembler_source_lang_test.cpp**, **disassembler_etsImplements_literal_test.cpp**, **disassembler_get_file_name_test.cpp**, **disassembler_line_number_release_test.cpp**.  
  - Many tests rely on `.abc` files generated from JS/TS under **tests/js/**, **tests/ts/**, **tests/module/**, **tests/sourceLang/**, **tests/annotations/**, etc., via **es2abc_gen_abc**.  
  - **.pa** sources in **tests/sources/** and expected outputs in **tests/expected/** for assembly-level tests.  
  - **.in** templates (e.g. **metadata_test.cpp.in**, **records_test.cpp.in**) used to generate test sources.  
  - **tests/BUILD.gn** defines test targets and abc generation for each test category.

---

## Build and test

### Build

- **Libraries**:  
  - **arkdisassembler** (shared; `.so` / `.dll` on Windows).  
  - **arkdisassembler_frontend_static** (static).  
- **Executable**: **ark_disasm** — links to **arkdisassembler_frontend_static**, reads input `.abc`, writes pandasm to a file.  
- **Config**: **arkdisassembler_public_config** defines **PANDA_WITH_ECMASCRIPT**.  
- **Dependencies**: **libarkassembler**, **libpandabase**, **libpandafile**; generated sources from **isa_gen_ark_disam** and **ark_gen_file** targets (type_to_pandasm_type, disasm_plugins_inc, get_language_specific_metadata_inc).  
- **Plugins**: BUILD.gn iterates **enabled_plugins** and adds plugin disassembler sources from **plugins/<plugin>/subproject_sources.gn** → **srcs_disassembler_path**; plugin YAMLs feed **disasm_plugins.inc** and **get_language_specific_metadata.inc**.

Typical build (from ark root):

```bash
./ark.py x64.release  # or the target that builds arkdisassembler / ark_disasm
```

### Test

- Tests are under **disassembler/tests/**; they link **arkdisassembler** (or static), use generated `.abc` and sometimes `.pa` + expected `.pa`.  
- Run the disassembler test suite via the project's test runner (e.g. the test targets defined in **tests/BUILD.gn**).

---

## How to add a new feature (checklist)

### A) New serialization or metadata behavior

1. Implement or extend the relevant **Serialize*** or **Get*Info** / **GetMetaData** methods in **disassembler.cpp**.  
2. If you need new fields on **ProgInfo**/ **RecordInfo**/ **MethodInfo**, add them in **accumulators.h** and populate them in **GetRecordInfo**/ **GetMethodInfo**/ **GetInsInfo**.  
3. Keep **disassembler.h** in sync (new public helpers if needed).  
4. Add or extend tests under **tests/** (reuse or add .abc/.pa/expected as needed).

### B) New bytecode instruction or opcode mapping

1. Instruction decoding and pandasm emission are **generated** from ISA (assembler + libpandafile ISAPI).  
2. Update the ISA description (e.g. **asm_isapi.rb**, **pandafile_isapi.rb**) so that the templates (**bc_ins_to_pandasm_ins.cpp.erb**, **opcode_translator.cpp.erb**, **get_ins_info.cpp.erb**) regenerate correctly.  
3. Rebuild and run **disassembler_imm_tests** and any instruction/opcode tests.  
4. Do **not** hand-edit generated **bc_ins_to_pandasm_ins.cpp**, **opcode_translator.cpp**, or **get_ins_info.cpp**.

### C) New type or literal handling

1. Literal/type handling is in **disassembler.cpp** (e.g. **FillLiteralData**, **FillLiteralArrayData**, **GetLiteralArrayByOffset**, **LiteralTagToString**) and in generated **type_to_pandasm_type.cpp**.  
2. For new Panda file types, update **libpandafile/types.yaml** and the type_to_pandasm_type generator if needed.  
3. For new literal tags or module literal formats, extend the switch/cases in **disassembler.cpp** and add tests (e.g. **literals_test**, **disassembler_module_literal_test**).

### D) Language- or plugin-specific metadata

1. Plugins register via **plugin_options.yaml** and the concatenated plugin YAMLs.  
2. Implement the plugin's disassembler part (e.g. under **plugins/<name>/disassembler/**) and list sources in the plugin's **subproject_sources.gn** under **srcs_disassembler_path**.  
3. Language-specific metadata is wired through **get_language_specific_metadata.inc.erb** (e.g. **Get<Directive>Metadata()**). Add or extend the template and plugin YAML so your plugin's getter is called from **GetLanguageSpecificMetadata()**.

---

## Critical pitfalls / "do not do this"

- **Do not** edit generated files under **$target_gen_dir** (e.g. **bc_ins_to_pandasm_ins.cpp**, **get_ins_info.cpp**, **opcode_translator.cpp**, **get_language_specific_metadata.inc**, **disasm_plugins.inc**). Change the `.erb` templates and/or ISA/plugin data, then regenerate.  
- **Do not** assume **ProgInfo** is populated before **CollectInfo()** is called; Serialize with **print_information == true** only makes sense after CollectInfo.  
- **Do not** open or serialize without checking that **file_** is non-null after **Disassemble()**; the CLI and callers should handle open failure.  
- **Do not** add new ISA-driven instruction handling in hand-written code when the instruction is already described in the ISA; extend the ISA and templates so all disassemblers stay in sync.  
- When adding plugin hooks, ensure the plugin is in **enabled_plugins** and that **disasm_plugins.inc** / **get_language_specific_metadata.inc** are regenerated so the build sees the new hooks.

---

## Where to look (quick reference)

| Goal | Location |
|------|----------|
| Public API and program/accumulator types | **disassembler.h**, **accumulators.h** |
| Implementation | **disassembler.cpp** |
| CLI | **disasm.cpp** |
| Generated instruction/opcode/type | **templates/*.erb**, ISA in **assembler/** and **libpandafile/** |
| Plugin wiring | **templates/disasm_plugins.inc.erb**, **templates/get_language_specific_metadata.inc.erb**, **plugin_options.yaml**, **plugins/*/subproject_sources.gn** |
| Build | **BUILD.gn** (arkdisassembler, ark_disasm, isa_gen_ark_disam, ark_gen_file targets) |
| Tests | **tests/*.cpp**, **tests/BUILD.gn**, **tests/sources/**, **tests/expected/**, **tests/js/**, **tests/ts/**, **tests/module/**, etc. |
