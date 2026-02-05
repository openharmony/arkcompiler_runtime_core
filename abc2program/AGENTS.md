# abc2program (runtime_core) — Agent Guide

This document is the **full working guide** for **`arkcompiler/runtime_core/abc2program`** (the **dynamic / runtime_core** abc2program).

There is a **separate** abc2program under **`arkcompiler/runtime_core/static_core/abc2program`** (static / ArkTS). That one has different API (e.g. **FillProgramData(program)** vs **CompileAbcFile()**), different key data (**Abc2ProgramKeyData** vs **Abc2ProgramEntityContainer**), different build targets (**arkts_abc2program**, **arkts_abc2prog** vs **abc2program**, **abc2prog**), and different dependencies (libarktsfile, libarktsassembler vs libpandafile, libarkassembler). Do not confuse the two.

---

## What is abc2program (runtime_core)?

**abc2program** converts **Panda/Ark binary bytecode** (`.abc` files) into an in-memory **`pandasm::Program`** — the same program representation used by the assembler, disassembler, and bytecode optimizer. It does **not** produce text assembly by default; that is done by **PandasmProgramDumper** when dumping to a `.pa` file.

Main capabilities:

- **ABC → Program**: Opens an `.abc` file and builds a **pandasm::Program** (record_table, function_table, literalarray_table, metadata, debug info). Used as a library by **panda_guard** and other tools that need to inspect or transform bytecode at the program level.
- **Per-record compilation**: Each class/record in the ABC is compiled by **AbcClassProcessor**, which fills one **pandasm::Record** (name, metadata, attributes, source_file, fields) and all its methods and literal arrays. Record names can be modified (bundle name, package name, version) when configured.
- **Bytecode → pandasm instructions**: Method bytecode is converted to **pandasm::Ins** via **AbcCodeConverter** (opcode and instruction conversion are partly generated from ISA). Labels, catch blocks, and local variable tables are reconstructed from code and debug info.
- **Dump to text**: **PandasmProgramDumper** can write the program to an output stream in pandasm text format (e.g. for the **abc2prog** CLI or debug dumps).

The **entry point** for the CLI is the **`abc2prog`** executable (`abc2prog_main.cpp`), which uses **Abc2ProgramDriver** to parse options, compile the ABC, and dump to a file. The core API is **Abc2ProgramCompiler** (OpenAbcFile, CompileAbcFile, CompileAbcClass, GetAbcFile, GetDebugInfoExtractor, version/name setters). The code is also built as **abc2program** (shared library) and **abc2program_frontend_static** (static library).

---

## High-level pipeline and data flow

1. **Open file**: **Abc2ProgramCompiler::OpenAbcFile(file_path)** opens the ABC with **panda_file::File::Open** and creates a **DebugInfoExtractor**. **file_** and **debug_info_extractor_** are held for the lifetime of the compiler.

2. **Compile** (**CompileAbcFile()**):
   - Allocate a new **pandasm::Program** (prog_); set **prog_->lang** to default source language.
   - For each class ID from **file_->GetClasses()**, call **CompileAbcClass(record_id, *prog_, record_name)**:
     - Build **Abc2ProgramEntityContainer** (file, program, debug_info_extractor, record_id, bundle_name); optionally set modify_pkg_name, package_name, origin_version, target_version.
     - **AbcClassProcessor(record_id, entity_container).FillProgramData()**.

3. **AbcClassProcessor::FillProgramData()**:
   - **FillRecord()**: Fill record name (via entity_container.GetFullRecordNameById), skip if array type; set metadata (access flags, language), **FillRecordData()** (metadata, **FillFields()**, source file), then insert into **program_->record_table**.
   - **FillFunctions()**: For each method in the class, **AbcMethodProcessor**.FillProgramData() → build **pandasm::Function** and add to **program_->function_table** (and function_synonyms as needed).
   - **FillLiteralArrayTable()**: **FillModuleLiteralArrays()**, **FillUnnestedLiteralArrays()**, **FillNestedLiteralArrays()** using **AbcLiteralArrayProcessor** / **AbcModuleArrayProcessor**; entity_container tracks module/unnested/nested literal array IDs.

4. **Method processing** (**AbcMethodProcessor**): Fill function name (signature), proto (params, return type via **AbcTypeConverter**), kind, metadata, attributes, **FillCodeData()** (via **AbcCodeProcessor**), debug info, annotations. **AbcCodeProcessor** uses **AbcCodeConverter::BytecodeInstructionToPandasmInstruction** (generated from ISA), builds instruction list with labels and catch blocks, and fills local variable table from debug info.

5. **Driver / CLI** (**Abc2ProgramDriver**): **Run(argc, argv)** parses **Abc2ProgramOptions** (input path, output path). **Compile(input_path)** opens the file and runs **CompileAbcFile()**; **Dump(output_path)** uses **PandasmProgramDumper** to write the program to a file.

Key data structures:

- **Abc2ProgramCompiler**: **file_**, **debug_info_extractor_**, **prog_** (owned; returned by CompileAbcFile); **bundle_name_**, **modify_pkg_name_**, **package_name_**, **origin_version_**, **target_version_** for name/version tweaks.
- **Abc2ProgramEntityContainer**: **file_**, **program_**, **debug_info_extractor_**, **current_class_id_**; **record_full_name_map_**, **method_full_name_map_**; **module_literal_array_id_set_**, **module_request_phase_id_set_**, **unnested_literal_array_id_set_**, **processed_nested_literal_array_id_set_**, **unprocessed_nested_literal_array_id_set_**; bundle/package/version strings.
- **pandasm::Program**: **record_table**, **function_table**, **literalarray_table**, **lang**, etc.

---

## Repository layout (where things live)

### Entry and driver

- **`abc2prog_main.cpp`**: **main** → **Abc2ProgramDriver::Run(argc, argv)**.
- **`abc2program_driver.h` / `abc2program_driver.cpp`**: **Abc2ProgramDriver** — **Run(argc, argv)** (parse options, then Run(input, output)); **Run(input_path, output_path)** (Compile + Dump); **Compile(input_path)** (OpenAbcFile, CompileAbcFile, store program); **Dump(output_path)** (PandasmProgramDumper); **GetProgram()**.
- **`abc2program_options.h` / `abc2program_options.cpp`**: **Abc2ProgramOptions** — **Parse(argc, argv)**; **GetInputFilePath()**, **GetOutputFilePath()**.

### Compiler and entity container

- **`abc2program_compiler.h` / `abc2program_compiler.cpp`**: **Abc2ProgramCompiler** — **OpenAbcFile**, **CompileAbcFile**, **CompileAbcClass**, **GetAbcFile**, **GetDebugInfoExtractor**, **CheckFileVersionIsSupported**, **CheckClassId**; setters for bundle_name, modify_pkg_name, package_name, origin_version, target_version.
- **`common/abc2program_entity_container.h` / `abc2program_entity_container.cpp`**: **Abc2ProgramEntityContainer** — constructor (file, program, debug_info_extractor, class_id, bundle_name); **GetStringById**, **GetFullRecordNameById**, **GetFullMethodNameById**; literal array ID sets and add/get helpers; **ModifyRecordName**, **ModifyPkgNameForRecordName**, **ModifyVersionForRecordName**, **ModifyPkgNameForFieldName**; **IsSourceFileRecord**; package/version setters.

### Processors (per-entity)

- **`abc_file_entity_processor.h`**: **AbcFileEntityProcessor** — base with **entity_id_**, **entity_container_**, **FillProgramData()** pure virtual.
- **`abc_class_processor.h` / `abc_class_processor.cpp`**: **AbcClassProcessor** — **FillProgramData()** (FillRecord, FillFunctions, FillLiteralArrayTable); FillRecord (name, metadata, attributes, source file, fields), FillFields, FillFunctions, FillLiteralArrayTable (module, unnested, nested).
- **`abc_method_processor.h` / `abc_method_processor.cpp`**: **AbcMethodProcessor** — **FillProgramData()** (FillFunctionData); FillProto, FillFunctionKind, FillFunctionMetaData, FillFunctionAttributes, FillCodeData (AbcCodeProcessor), FillDebugInfo, FillFuncAnnotation.
- **`abc_code_processor.h` / `abc_code_processor.cpp`**: **AbcCodeProcessor** — **FillProgramData()** (regs_num, instructions with labels, catch blocks, local variable table, debug); uses **AbcCodeConverter** for bytecode→pandasm.
- **`abc_field_processor.h` / `abc_field_processor.cpp`**: **AbcFieldProcessor** — fills a single **pandasm::Field** for a record.
- **`abc_annotation_processor.h` / `abc_annotation_processor.cpp`**: **AbcAnnotationProcessor** — annotation data for records/methods.
- **`abc_literal_array_processor.h` / `abc_literal_array_processor.cpp`**: **AbcLiteralArrayProcessor** — literal array content.
- **`abc_module_array_processor.h` / `abc_module_array_processor.cpp`**: **AbcModuleArrayProcessor** — module literal arrays.

### Code and type conversion (common + generated)

- **`common/abc_code_converter.h`** + implementation (generated): **AbcCodeConverter** — **BytecodeInstructionToPandasmInstruction**, **BytecodeOpcodeToPandasmOpcode**, **IDToString**; uses entity_container for string/ID resolution.
- **`common/abc_type_converter.h`** + generated **abc_type_convert.cpp**: **AbcTypeConverter** — Panda types to pandasm types (from **libpandafile/types.yaml**).
- **`common/abc_file_utils.h` / `abc_file_utils.cpp`**: **AbcFileUtils** — e.g. **IsArrayTypeName**.
- Generated under **$target_gen_dir** from BUILD.gn: **abc_inst_convert.cpp**, **abc_opcode_convert.cpp** (from **common/abc_inst_convert.cpp.erb**, **common/abc_opcode_convert.cpp.erb** via **isa_gen_abc2program**); **abc_type_convert.cpp** (from **common/abc_type_convert.cpp.erb** + **libpandafile/types.yaml**); **program_dump_utils.cpp** (from **common/program_dump_utils.cpp.erb**).

### Dump

- **`program_dump.h` / `program_dump.cpp`**: **PandasmProgramDumper** — **Dump(ostream, program)**; **SetAbcFilePath**; dumps language, literal array table, record table, function table, strings; optional normalized and debug modes; uses **dump_utils** and generated **program_dump_utils**.
- **`dump_utils.h` / `dump_utils.cpp`**: **LiteralTagToStringMap**, **LabelMap**, **FunctionKindToStringMap**, **OpcodeLiteralIdIndexMap**; dump title constants; label/instruction mapping helpers.

### Log and tests

- **`abc2program_log.h` / `abc2program_log.cpp`**: Logging / unimplemented helpers.
- **`tests/BUILD.gn`**: Test targets — C++ tests (e.g. **abc2program_test_utils**, **hello_world_test**, **js_common_syntax_test**, **debug_line_number**, **release_line_number**, **release_column_number**, **etsImplements_test**, **sourceLang_test**); ABCs generated from **tests/js/** and **tests/ts/** (and subdirs) via **es2abc_gen_abc**; expected dump files in **tests/js/**, **tests/ts/** (e.g. *DumpExpected.txt, *DebugDumpExpected.txt).

---

## Build and test

### Build

- **Libraries**: **abc2program** (shared), **abc2program_frontend_static** (static) — all compiler and dump sources plus generated **abc_inst_convert.cpp**, **abc_opcode_convert.cpp**, **abc_type_convert.cpp**, **program_dump_utils.cpp**. Deps: **isa_gen_abc2program_***, **abc_type_convert_cpp**, **libarkassembler** (static or frontend_static), and for static **libpandabase**, **libpandafile**. Config: **abc2program_public_config** (defines **PANDA_WITH_ECMASCRIPT**).
- **Executable**: **abc2prog** — links **abc2program_frontend_static**; **install_enable = false**.

From ark root:

```bash
./ark.py x64.release  # or the target that builds abc2program / abc2prog
```

### Test

- Tests are under **tests/**; they link **abc2program** (or static), use ABCs generated from **tests/js/** and **tests/ts/** (and subdirs) via **es2abc_gen_abc**, and compare dumps against expected `.txt` files. Run via the project test runner.

---

## How to add a new feature (checklist)

### A) New record or method metadata

1. In **AbcClassProcessor** or **AbcMethodProcessor**, add a **FillXxx()** step (e.g. after **FillRecordMetaData** or **FillFunctionMetaData**) that reads from **ClassDataAccessor** / **MethodDataAccessor** (or debug_info_extractor) and sets the corresponding **pandasm::Record** or **pandasm::Function** field.
2. If the new data affects **Abc2ProgramEntityContainer** (e.g. name resolution), extend the container and use it in the processor. Keep **GetFullRecordNameById** / **GetFullMethodNameById** and literal-array ID logic consistent.
3. If the dumper must emit the new data, extend **PandasmProgramDumper** (e.g. **DumpRecord** / **DumpFunction** or a new section) and **dump_utils** constants if needed. Add tests that compare dump output to an expected file.

### B) New bytecode instruction or opcode

1. Instruction and opcode conversion are **generated** from ISA (assembler + libpandafile). Update the ISA description so that **abc_inst_convert.cpp.erb** and **abc_opcode_convert.cpp.erb** produce the new cases; do not hand-edit the generated **abc_inst_convert.cpp** / **abc_opcode_convert.cpp**.
2. If **AbcCodeConverter** or **AbcCodeProcessor** has hand-written fallbacks or special cases, add the new instruction there only if it cannot be expressed via the ISA templates. Regenerate and run code-related tests (e.g. release-line-number, debug-line-number).

### C) New literal array or module format

1. Extend **AbcLiteralArrayProcessor** or **AbcModuleArrayProcessor** to decode the new format and fill **program_->literalarray_table** (or module arrays). Use **Abc2ProgramEntityContainer**'s literal array ID sets so that module/unnested/nested ordering and dependencies stay correct.
2. If the new format affects **GetLiteralArrayIdName** or how literal array IDs are resolved in instructions, update **entity_container** and **AbcCodeConverter**/generated code accordingly. Add tests with ABCs that contain the new literal form and expected dumps.

### D) New compiler option or name/version rule

1. Add the option to **Abc2ProgramOptions** (and CLI parsing) or pass it into **Abc2ProgramCompiler** via a new setter. Use it in **Abc2ProgramCompiler::CompileAbcClass** (e.g. when building **Abc2ProgramEntityContainer** or when calling **ModifyRecordName** / **ModifyPkgNameForRecordName** / **ModifyVersionForRecordName**).
2. If the option changes dump output (e.g. normalized vs non-normalized), extend **PandasmProgramDumper** (e.g. constructor or **SetAbcFilePath**-style setter) and the driver so the dump mode is passed through. Add a test that runs the compiler/driver with the new option and checks the output.

---

## Critical pitfalls / "do not do this"

- **Do not** edit generated files (**abc_inst_convert.cpp**, **abc_opcode_convert.cpp**, **program_dump_utils.cpp**, **abc_type_convert.cpp**) by hand. Change the `.erb` templates and/or **types.yaml** and regenerate.
- **Do not** assume **file_** is non-null after **OpenAbcFile**; callers (e.g. panda_guard) check the return value. **CompileAbcFile()** allocates and returns **prog_**; the compiler keeps ownership and deletes it in the destructor — callers that use **CompileAbcFile()** typically **std::move(*compiler.CompileAbcFile())** into their own **pandasm::Program**.
- **Do not** add a new record or function to **program_** without going through the same **record_table** / **function_table** and (where applicable) **function_synonyms** conventions, or the assembler and other tools may reject or misinterpret the program.
- **Do not** change the order of literal array processing (module → unnested → nested) or the semantics of **unprocessed_nested_literal_array_id_set_** without updating all processors that depend on it; literal array IDs are resolved and emitted in a specific order.
- When adding dump output, ensure **PandasmProgramDumper**'s **program_** pointer and **abc_file_path_** are set correctly (e.g. via **Dump(ofs, program)** and **SetAbcFilePath**), and that normalized/debug flags match the test expectations.

---

## Where to look (quick reference)

| Goal | Location |
|------|----------|
| Entry and driver | **abc2prog_main.cpp**, **abc2program_driver.h**, **abc2program_driver.cpp**, **abc2program_options.h** |
| Compiler and container | **abc2program_compiler.h**, **abc2program_compiler.cpp**, **common/abc2program_entity_container.h** |
| Processors | **abc_class_processor.h**, **abc_class_processor.cpp**, **abc_method_processor.h**, **abc_code_processor.h**, **abc_field_processor.h**, **abc_literal_array_processor.h**, **abc_module_array_processor.h**, **abc_file_entity_processor.h** |
| Code/type conversion | **common/abc_code_converter.h**, **common/abc_type_converter.h**, **common/abc_inst_convert.cpp.erb**, **common/abc_opcode_convert.cpp.erb**, **common/abc_type_convert.cpp.erb** |
| Dump | **program_dump.h**, **program_dump.cpp**, **dump_utils.h**, **dump_utils.cpp** |
| Build | **BUILD.gn** (abc2program, abc2program_frontend_static, abc2prog, isa_gen_abc2program, abc_type_convert_cpp) |
| Tests | **tests/BUILD.gn**, **tests/cpp_sources/** , **tests/js/** , **tests/ts/** , expected `.txt` files |
