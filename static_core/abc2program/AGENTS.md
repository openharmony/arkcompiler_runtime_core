# abc2program (static_core) — Agent Guide

This document is the **full working guide** for **`arkcompiler/runtime_core/static_core/abc2program`**.

---

## What is abc2program (static_core)?

**abc2program (static_core)** converts **Panda/Ark binary bytecode** (`.abc` files) into an in-memory **`pandasm::Program`** for the **static VM / ArkTS** toolchain. It is used when building or inspecting ABCs in the static runtime context (e.g. ArkTS compiler pipeline, libabckit when enabled).

This is a **different** implementation from **`arkcompiler/runtime_core/abc2program`** (the dynamic / runtime_core abc2program). The two use different namespaces (`ark::abc2program` vs `panda::abc2program`), different key data (Abc2ProgramKeyData vs Abc2ProgramEntityContainer), different build targets (arkts_abc2prog / arkts_abc2program vs abc2prog / abc2program), and different dependencies (libarktsfile, libarktsassembler vs libpandafile, libarkassembler).

Main capabilities:

- **ABC → Program**: Opens an `.abc` file and fills a **pandasm::Program** (record_table, function_table, literalarray_table, strings, metadata). The compiler does **not** own the program; the caller passes it in and **FillProgramData(program)** fills it.
- **File-level processing**: A single **AbcFileProcessor** processes the whole file: **ProcessClasses()** (one **AbcClassProcessor** per class, which fills one record and its fields/methods), **FillLiteralArrays()**, **FillProgramStrings()**, **FillExternalFieldsToRecords()**, **GetLanguageSpecificMetadata()** (plugin-generated).
- **Key data**: **Abc2ProgramKeyData** holds the file, **AbcStringTable**, and program; **recordNameToId_**, **methodNameToId_**, **externalFieldTable_**; **GetFullRecordNameById**, **GetFullFunctionNameById**, **GetLiteralArrayIdName**, file language. **AbcStringTable** tracks string IDs used and provides **GetStringById** / **GetStringSet** for dump.
- **Bytecode → pandasm**: Method bytecode is converted via **AbcCodeConverter** (generated from ISA: abc_inst_convert, abc_opcode_convert). Types from **AbcTypeConverter** (generated from libarkfile types.yaml).
- **Dump**: **PandasmProgramDumper** takes the **panda_file::File** and **AbcStringTable** and dumps the program to an output stream (e.g. for **arkts_abc2prog** CLI).

The **entry point** for the CLI is **`arkts_abc2prog`** (`abc2prog_main.cpp`), which uses **Abc2ProgramDriver** to parse options, **Compile(input_path, program_)** (OpenAbcFile + FillProgramData), and **Dump(output_path)**. The core API is **Abc2ProgramCompiler** (OpenAbcFile, GetAbcFile, GetAbcStringTable, FillProgramData). Built as **arkts_abc2program** (shared) and **arkts_abc2program_static** (static). **Plugin-based**: sources include **abc2program_plugins.inc** and **get_language_specific_metadata.inc** (from plugin YAMLs).

---

## High-level pipeline and data flow

1. **Open file**: **Abc2ProgramCompiler::OpenAbcFile(filePath)** opens the ABC and creates **AbcStringTable(*file_)**. **file_** and **stringTable_** are held for the lifetime of the compiler.

2. **Fill program** (**FillProgramData(program)**):
   - Build **Abc2ProgramKeyData(*file_, *stringTable_, program)**.
   - **AbcFileProcessor(keyData).ProcessFile()**.

3. **AbcFileProcessor::ProcessFile()**:
   - **ProcessClasses()**: For each class ID, get record language (update keyData file language if needed), create **AbcClassProcessor(recordId, keyData)** — constructor calls **FillProgramData()** which does **FillRecord()** (name, metadata, record data: fields, **FillFunctions()**, source file, annotations), then insert into **program_->recordTable** and **keyData_.GetRecordNameToIdTable()**.
   - **FillLiteralArrays()**: **AbcLiteralArrayProcessor(file_->GetLiteralArraysId(), keyData)** (fills literalarray_table).
   - **FillProgramStrings()**: **program_->strings = stringTable_->GetStringSet()**.
   - **FillExternalFieldsToRecords()**: Attach external fields from **keyData_.GetExternalFieldTable()** to records.
   - **GetLanguageSpecificMetadata()**: Generated; calls plugin language-specific metadata.
   - **program_->lang = keyData_.GetFileLanguage()**.

4. **AbcClassProcessor**: Constructor calls **FillProgramData()** → **FillRecord()** (record name, **FillRecordMetaData**, **FillRecordData** for non-external: **FillFields**, **FillFunctions**, **FillRecordSourceFile**, **FillRecordAnnotations**). **FillFunctions()** creates **AbcMethodProcessor(methodId, keyData)** per method (constructor fills function and adds to program).

5. **Driver**: **Compile(inputPath)** → **compiler_.OpenAbcFile** then **compiler_.FillProgramData(program_)**. **Dump(outputPath)** → **PandasmProgramDumper(compiler_.GetAbcFile(), compiler_.GetAbcStringTable()).Dump(ofs, program_)**.

Key data structures:

- **Abc2ProgramCompiler**: **file_**, **stringTable_** (AbcStringTable), **keyData_** (created in FillProgramData).
- **Abc2ProgramKeyData**: **file_**, **stringTable_**, **program_**; **recordNameToId_**, **methodNameToId_**, **externalFieldTable_**; **fileLanguage_**; **GetFullRecordNameById**, **GetFullFunctionNameById**, **GetLiteralArrayIdName**.
- **AbcStringTable**: **file_**, **stingIdSet_** (typo in code: string IDs); **GetStringById**, **AddStringId**, **GetStringSet**, **Dump**.
- **AbcFileProcessor**: Includes **abc2program_plugins.inc** (plugin-specific members); **ProcessFile** orchestrates classes, literal arrays, strings, external fields, language-specific metadata.
- **pandasm::Program**: **recordTable**, **functionTable**, **literalarray_table**, **strings**, **lang**.

---

## Repository layout (where things live)

### Entry and driver

- **`abc2prog_main.cpp`**: **main** → **Abc2ProgramDriver::Run(argc, argv)**.
- **`abc2program_driver.h` / `abc2program_driver.cpp`**: **Abc2ProgramDriver** — **Run(argc, argv)** (parse options, Run(input, output)); **Compile(inputPath)** (delegates to Compile(inputPath, program_)), **Compile(inputPath, program)** (OpenAbcFile, FillProgramData); **Dump(outputPath)** (PandasmProgramDumper with GetAbcFile(), GetAbcStringTable()); **GetProgram()**.
- **`abc2program_options.h` / `abc2program_options.cpp`**: **Abc2ProgramOptions** — Parse, GetInputFilePath, GetOutputFilePath.

### Compiler and key data

- **`abc2program_compiler.h` / `abc2program_compiler.cpp`**: **Abc2ProgramCompiler** — **OpenAbcFile**, **GetAbcFile**, **GetAbcStringTable**, **FillProgramData(program)** (builds KeyData, AbcFileProcessor::ProcessFile). Does **not** own the program.
- **`abc2program_key_data.h` / `abc2program_key_data.cpp`**: **Abc2ProgramKeyData** — file, stringTable, program; recordNameToId_, methodNameToId_, externalFieldTable_; GetFullRecordNameById, GetFullFunctionNameById, GetLiteralArrayIdName, GetFileLanguage, SetFileLanguage; **GetFunctionTable**, **GetExternalFieldTable**, **GetMethodNameToIdTable**, **GetRecordNameToIdTable**.
- **`abc_string_table.h` / `abc_string_table.cpp`**: **AbcStringTable** — GetStringById, AddStringId, GetStringSet, StringDataToString, Dump.

### File and class processing

- **`abc_file_processor.h` / `abc_file_processor.cpp`**: **AbcFileProcessor** — **ProcessFile()** (ProcessClasses, FillLiteralArrays, FillProgramStrings, FillExternalFieldsToRecords, GetLanguageSpecificMetadata); **ProgAnnotations**, **RecordAnnotations**; includes **abc2program_plugins.inc**; **GetRecordLanguage**, annotation/value helpers.
- **`abc_file_entity_processor.h` / `abc_file_entity_processor.cpp`**: **AbcFileEntityProcessor** — base with **entityId_**, **keyData_**, **file_**, **stringTable_**, **program_**; **SetEntityAttribute** / **SetEntityAttributeValue** templates.
- **`abc_class_processor.h` / `abc_class_processor.cpp`**: **AbcClassProcessor** — constructor calls **FillProgramData()**; **FillRecord** (name, metadata, record data: fields, functions, source file, annotations), **FillFields**, **FillFunctions**, **FillRecordSourceFile**, **FillRecordAnnotations**.
- **`abc_method_processor.h` / `abc_method_processor.cpp`**: **AbcMethodProcessor** — fills **pandasm::Function** and adds to program (constructor likely calls FillProgramData).
- **`abc_field_processor.h` / `abc_field_processor.cpp`**: **AbcFieldProcessor** — fills field(s) for a record.
- **`abc_annotation_processor.h` / `abc_annotation_processor.cpp`**: **AbcAnnotationProcessor** — annotation data.
- **`abc_literal_array_processor.h` / `abc_literal_array_processor.cpp`**: **AbcLiteralArrayProcessor** — fills literalarray_table from file literal arrays.
- **`abc_file_utils.h` / `abc_file_utils.cpp`**: File/type helpers (e.g. **IsArrayTypeName**).

### Code and type conversion (common + generated)

- **`common/abc_code_converter.h` / `common/abc_code_converter.cpp`**: **AbcCodeConverter** — BytecodeInstructionToPandasmInstruction, BytecodeOpcodeToPandasmOpcode, IDToString (non-generated part).
- **`common/abc_type_converter.h`** + generated **abc_type_convert.cpp**: **AbcTypeConverter** (from libarkfile types.yaml).
- Generated under **$target_gen_dir**: **abc_inst_convert.cpp**, **abc_opcode_convert.cpp** (from **templates/abc_inst_convert.cpp.erb**, **templates/abc_opcode_convert.cpp.erb** via **isa_gen_abc2program**); **abc_type_convert.cpp** (ark_gen_file, template from **$ark_root/abc2program/templates/abc_type_convert.cpp.erb**, data libarkfile types.yaml).

### Dump and plugins

- **`program_dump.h` / `program_dump.cpp`**: **PandasmProgramDumper** — constructor takes **(const panda_file::File &file, const AbcStringTable &stringTable)**; **Dump(ostream, program)**; dumps path, language, literal array table (with/without key), record table, function table, instructions, strings (by string table or by program).
- **`abc2program_log.h` / `abc2program_log.cpp`**: Logging.
- **`templates/abc2program_plugins.inc.erb`**: Generated **abc2program_plugins.inc** (included in **AbcFileProcessor**; plugin-specific members).
- **`templates/get_language_specific_metadata.inc.erb`**: Generated **get_language_specific_metadata.inc** (implements **GetLanguageSpecificMetadata** from plugin YAMLs).

---

## Build and test

### Build

- **Libraries**: **arkts_abc2program** (shared), **arkts_abc2program_static** (static) — **abc2program_sources** (compiler, driver, key_data, string_table, file/class/method/field/annotation/literal_array processors, file_utils, common/code_converter, program_dump) + **plugin_abc2program_sources** + generated **abc_inst_convert.cpp**, **abc_opcode_convert.cpp**, **abc_type_convert.cpp**. Deps: **abc2program_plugins_inc**, **get_language_specific_metadata_inc**, **abc_type_convert_cpp**, **isa_gen_abc2program_***, **libarktsassembler**, **libarktsbase**, **libarktsfile**. Config: **arkts_abc2program_public_config** (include_dirs $ark_root/abc2program, $target_gen_dir; **ENABLE_LIBABCKIT** when abckit_enable).
- **Executable**: **arkts_abc2prog** — links **arkts_abc2program**; **install_enable = false**.
- **libarktsabc2program_package**: Optional package that pulls in arkts_abc2program/arkts_abc2program_static when **enable_static_vm**.

From ark root (with static VM enabled as needed):

```bash
./ark.py x64.release  # or the target that builds arkts_abc2program / arkts_abc2prog
```

### Test

- Tests for static_core abc2program live under **static_core/plugins/ets/tests/abc2program/** (e.g. **abc2program_test.cpp**, ETS sources). They use the static_core build and **arkts_abc2program** (or static). Run via the project test runner.

---

## How to add a new feature (checklist)

### A) New record or method metadata

1. In **AbcClassProcessor** or **AbcMethodProcessor**, add a **FillXxx()** step that reads from **ClassDataAccessor** / **MethodDataAccessor** and sets the **pandasm::Record** or **pandasm::Function** field. Use **keyData_** for name/ID resolution (**GetFullRecordNameById**, **GetFullFunctionNameById**, **stringTable_->GetStringById**).
2. If the new data affects **Abc2ProgramKeyData** (e.g. new name→id map), extend **Abc2ProgramKeyData** and update **AbcFileProcessor** / class processor to fill it. If the dumper must emit it, extend **PandasmProgramDumper** and add tests.

### B) New bytecode instruction or opcode

1. Instruction/opcode conversion are **generated** from ISA. Update the ISA so **templates/abc_inst_convert.cpp.erb** and **templates/abc_opcode_convert.cpp.erb** produce the new cases. Do not hand-edit generated **abc_inst_convert.cpp** / **abc_opcode_convert.cpp**.
2. If **AbcCodeConverter** has hand-written logic, add the new case there only if it cannot be expressed in the templates. Regenerate and run tests.

### C) New literal array or string handling

1. Extend **AbcLiteralArrayProcessor** or **AbcStringTable** (e.g. new literal tag or string usage). Use **keyData_.GetLiteralArrayIdName** and **keyData_.GetAbcStringTable()** so names and string sets stay consistent. Update **PandasmProgramDumper** if dump format changes (e.g. **DumpLiteralArrayTableWithKey** / **WithoutKey**, **DumpStringsByStringTable** / **ByProgram**).
2. Add tests under **plugins/ets/tests/abc2program** with ABCs that exercise the new format.

### D) Plugin or language-specific metadata

1. Plugin sources are wired via **plugin_abc2program_sources** and **templates/abc2program_plugins.inc.erb** / **get_language_specific_metadata.inc.erb**. Add or extend the plugin YAML and template so your plugin's members and **GetLanguageSpecificMetadata** hooks are generated and called from **AbcFileProcessor::GetLanguageSpecificMetadata()**.
2. Ensure the plugin is in **ark_plugin_options_yaml** (or equivalent) so the generated inc files are up to date.

---

## Critical pitfalls / "do not do this"

- **Do not** mix code or headers from **runtime_core/abc2program** (panda::abc2program, Abc2ProgramEntityContainer, CompileAbcFile returning Program*) with **static_core/abc2program** (ark::abc2program, Abc2ProgramKeyData, FillProgramData(program)). They are separate implementations with different APIs and data flow.
- **Do not** edit generated files (**abc_inst_convert.cpp**, **abc_opcode_convert.cpp**, **abc_type_convert.cpp**, **abc2program_plugins.inc**, **get_language_specific_metadata.inc**) by hand. Change the `.erb` templates and/or plugin YAMLs and regenerate.
- **Do not** assume **file_** is non-null after **OpenAbcFile**; callers check the return value. The compiler does **not** own the **pandasm::Program**; the caller passes it in and retains ownership.
- **Do not** add records/functions to **program_** outside the existing **AbcClassProcessor** / **AbcMethodProcessor** flow without updating **keyData_** (recordNameToId_, methodNameToId_, externalFieldTable_) so that dumper and downstream tools stay consistent.
- When changing **PandasmProgramDumper**, remember it takes **(file, stringTable)** and uses **stringTable** for string dumping (e.g. **DumpStringsByStringTable**); **program_->strings** is filled from **stringTable_->GetStringSet()** in **FillProgramStrings()**.

---

## Where to look (quick reference)

| Goal | Location |
|------|----------|
| Entry and driver | **abc2prog_main.cpp**, **abc2program_driver.h**, **abc2program_driver.cpp**, **abc2program_options.h** |
| Compiler and key data | **abc2program_compiler.h**, **abc2program_compiler.cpp**, **abc2program_key_data.h**, **abc_string_table.h** |
| File and class processing | **abc_file_processor.h**, **abc_file_processor.cpp**, **abc_class_processor.h**, **abc_file_entity_processor.h**, **abc_method_processor.h**, **abc_field_processor.h**, **abc_literal_array_processor.h**, **abc_annotation_processor.h**, **abc_file_utils.h** |
| Code/type conversion | **common/abc_code_converter.h**, **common/abc_type_converter.h**, **templates/abc_inst_convert.cpp.erb**, **templates/abc_opcode_convert.cpp.erb** |
| Dump and plugins | **program_dump.h**, **program_dump.cpp**, **templates/abc2program_plugins.inc.erb**, **templates/get_language_specific_metadata.inc.erb** |
| Build | **BUILD.gn** (arkts_abc2program, arkts_abc2program_static, arkts_abc2prog, isa_gen_abc2program, abc_type_convert_cpp, abc2program_plugins_inc, get_language_specific_metadata_inc) |
| Tests | **static_core/plugins/ets/tests/abc2program/** (e.g. abc2program_test.cpp, ets sources) |
