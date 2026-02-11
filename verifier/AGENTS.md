# verifier (runtime_core) — Agent Guide

This document is the **full working guide** for **`arkcompiler/runtime_core/verifier`** (the **runtime_core** / structural verifier).

There is a **separate** verification framework under **`arkcompiler/runtime_core/static_core/verification`** (static / ArkTS). That one performs **semantic and type verification** (control flow, type resolution, abstract interpretation). The **runtime_core verifier** does **not** do that; it focuses on **structural and referential integrity** of `.abc` files (checksum, constant pool, literal arrays, register indices, instructions). Do not confuse the two.

---

## What is the verifier (runtime_core)?

The **verifier** is a **Panda/Ark bytecode validation tool** for `.abc` files. It checks that a binary ABC file is well-formed and that its constant pool, instructions, and metadata are consistent. It does **not** perform full semantic or type verification (that is done by **static_core/verification**); it focuses on structural and referential integrity.

Main capabilities:

- **Checksum**: Verifies the file header checksum (Adler-32 over file content after the header).
- **Constant pool**: Ensures constant-pool indices are in bounds and that all method/literal/string IDs referenced in bytecode and literal arrays are valid and mutually consistent.
- **Literal arrays**: Validates each literal array (method refs, string refs, nested literal refs, tagged values, impure NaN checks).
- **Register index** (optional/public for tests): Ensures every instruction's vreg indices are within the method's valid register range (vregs + args), including range instructions.
- **Instructions**: Primary opcode validity, jump target validity, try/catch block structure.

The **entry point** is the **ark_verifier** executable (**main.cpp**), which parses the input file path and calls **Verify(input_file)**. The core logic lives in **panda::verifier::Verifier** and can also be used via the **libarkverifier** shared library.

---

## High-level pipeline and data flow

1. **Open file**: **Verifier(filename)** opens the ABC with **panda_file::File::Open** and stores it in **file_**.

2. **Verify()** (main entry):
   - **VerifyChecksum()**: Compare **file_->GetHeader()->checksum** with Adler-32 of file content from **FILE_CONTENT_OFFSET** to end. Return false on mismatch.
   - **CollectIdInfos()**: Build ID sets used for later checks:
     - **GetConstantPoolIds()**: Fill **constant_pool_ids_** from the file's method index (constant pool region).
     - **GetLiteralIds()** (if **include_literal_array_ids**): Fill **literal_ids_** from literal arrays in the file.
     - **CheckConstantPool(ActionType::COLLECTINFOS)**: For each non-external class and method, call **CollectIdInInstructions** to collect **ins_method_ids_**, **ins_literal_ids_**, **ins_string_ids_** from bytecode; collect **all_method_ids_**; for fields, **CollectModuleLiteralId** into **module_literals_**.
   - **VerifyConstantPool()**:
     - **CheckConstantPoolIndex()**: Ensure constant-pool-related indices are within file bounds.
     - **CheckConstantPool(ActionType::CHECKCONSTPOOLCONTENT)**: For each method, **CheckConstantPoolMethodContent** — verify that every method/literal/string ID used in instructions is valid (**VerifyMethodId**, **VerifyLiteralId**, **VerifyStringId**).
     - **VerifyLiteralArrays()**: For each literal array, **VerifySingleLiteralArray** — validate method/string/literal refs and tagged double encoding (impure NaN check).

3. **Optional / test-only entry points** (not run in the default **Verify()** path but exposed for unit tests):
   - **VerifyRegisterIndex()**: For each method, iterate instructions and **CheckVRegIdx** (vreg count, range instructions, bounds).
   - **VerifyConstantPoolIndex()**: Only **CheckConstantPoolIndex()**.
   - **VerifyConstantPoolContent()**: **CheckConstantPool(CHECKCONSTPOOLCONTENT)** + **VerifyLiteralArrays()**.

Key data structures:

- **file_**: **std::unique_ptr<const panda_file::File>** — the opened ABC.
- **constant_pool_ids_**: Method IDs from the constant pool region.
- **literal_ids_**: Literal array entity IDs (from header or CollectUtil).
- **all_method_ids_**: All method IDs encountered when iterating classes/methods.
- **ins_method_ids_**, **ins_literal_ids_**, **ins_string_ids_**: IDs referenced in bytecode (from COLLECTINFOS).
- **module_literals_**: Literal IDs used as module field values.
- **instruction_index_map_**: Precomputed instruction pointer → index for jump/try verification.
- **MethodInfos**: Bundles bytecode pointers, method accessor, and out-parameters for **VerifyMethodInstructions** (register count, slot number, etc.).

---

## Repository layout (where things live)

### Entry and public API

- **main.cpp**: **ark_verifier** executable — **PandArgParser** (help, input_file), then **Verify(input_file)**; exit 0 on success, 1 on failure.
- **verify.h** / **verify.cpp**: **Verify(const std::string &input_file)** — constructs **Verifier**, returns **Verifier::Verify()**. Thin wrapper for the library/executable API.
- **verifier.h**: **panda::verifier::Verifier** — constructor taking filename; **Verify()**, **CollectIdInfos()**, **VerifyChecksum()**, **VerifyConstantPool()**, **VerifyRegisterIndex()**, **VerifyConstantPoolIndex()**, **VerifyConstantPoolContent()**; **MethodInfos**; constants (TAG_BITS_SIZE, MAX_REGISTER_INDEX, etc.); optional **include_literal_array_ids**, **literal_ids_**, **inner_literal_map_**, **inner_method_map_**. Private helpers for ID collection, constant-pool checks, literal-array checks, register-index checks, jump/try-catch verification.
- **verifier.cpp**: Implements all Verifier methods — checksum, GetConstantPoolIds/GetLiteralIds, CheckConstantPool (COLLECTINFOS / CHECKCONSTPOOLCONTENT), CollectIdInInstructions, VerifyLiteralArrays, VerifySingleLiteralArray, VerifyMethodId/VerifyLiteralId/VerifyStringId, VerifyRegisterIndex, CheckVRegIdx, GetVRegCount, jump and try-block verification, slot number from annotation, etc.

### Tests

- **tests/BUILD.gn**: Defines **VerifierTest** (host_unittest_action) — C++ sources: **utils.cpp**, **verify_checksum_test.cpp**, **verify_constant_pool_tests.cpp**, **verify_register_index_test.cpp**. Deps: **libarkverifier**, libpandabase, libpandafile. Generates ABCs from **tests/js/** and **tests/ts/** via **es2abc_gen_abc**. **GRAPH_TEST_ABC_DIR** points to the directory containing the generated .abc files.
- **tests/utils.h** / **tests/utils.cpp**: Test helpers.
- **tests/config.py**: Shared config for Python tests.
- **tests/verify_262abc_files.py**, **tests/verify_es2panda_test_abc.py**, **tests/verify_sys_hap_abc.py**, **tests/version_compatibility_test.py**: Python-based verification tests.
- **tests/js/*.js**, **tests/ts/*.ts**: Source files compiled to .abc for C++ unit tests.

---

## Build and test

### Build

- **Executable**: **ark_verifier** — sources: **main.cpp**, **verifier.cpp**, **verify.cpp**. Deps: **libpandafile:libarkfile_static_verifier**. Config: **arkverifier_public_config**. **install_enable = true**.
- **Library**: **libarkverifier** (shared) — sources: **verifier.cpp**, **verify.cpp**; same deps and config. Used by tests and potentially by other tools.

From ark root:

```bash
./ark.py x64.release  # or the target that builds ark_verifier / libarkverifier
```

### Test

- **VerifierTest**: Host unit tests; run via the project test runner. Tests use ABCs generated from **tests/js/** and **tests/ts/**; **GRAPH_TEST_ABC_DIR** must point to the output directory so tests can find e.g. **test_checksum.abc**, **test_constant_pool.abc**, etc.
- Python tests: Run with the project's Python test runner if configured (e.g. verify_262abc_files, verify_es2panda_test_abc, verify_sys_hap_abc, version_compatibility_test).

---

## How to add a new feature (checklist)

### A) New structural check (e.g. another pool or header field)

1. Add a new **VerifyXxx()** method in **verifier.h** / **verifier.cpp** that takes **file_** and optional precomputed IDs. Return bool; log with **LOG(ERROR, VERIFIER)** on failure.
2. Call the new check from **Verify()** in the appropriate order (e.g. after checksum, before or after constant pool). If the check needs **CollectIdInfos** data, run it after **CollectIdInfos()**.
3. Add unit tests under **tests/** that build a small .abc (or use an existing generated one) and expect pass/fail; add a JS/TS source and **es2abc_gen_abc** if you need a new ABC fixture.

### B) New instruction or bytecode property check

1. Extend the iteration in **CollectIdInInstructions** or **VerifyMethodInstructions** (or the method that walks instructions for register index) to detect the new case. Use **BytecodeInstruction** format/flags and existing helpers (**GetFirstImmFromInstruction**, **GetVRegCount**, etc.).
2. If you need a precomputed instruction index map, use **PrecomputeInstructionIndices** and **instruction_index_map_** as in jump/try verification.
3. Add tests that trigger the new check (valid and invalid cases) via small JS/TS or hand-crafted ABC if necessary.

### C) New constant-pool or literal-array rule

1. In **CheckConstantPoolMethodContent** or **VerifySingleLiteralArray** (and related **VerifyMethodIdInLiteralArray**, **VerifyStringIdInLiteralArray**, **VerifyLiteralIdInLiteralArray**), add the new rule. Reuse **constant_pool_ids_**, **literal_ids_**, **all_method_ids_**, **ins_*_ids_**, **module_literals_** so that cross-references stay consistent.
2. If the rule depends on a new annotation or metadata, add parsing (e.g. **GetSlotNumberFromAnnotation**-style) and document the expected format.
3. Add unit tests that pass/fail according to the new rule; extend **verify_constant_pool_tests** or add a new test file.

### D) New CLI option or library option

1. In **main.cpp**, add a **PandArg** and pass it into **Verify** or a new overload. If the option affects **Verifier** behavior (e.g. **include_literal_array_ids**), pass it into **Verifier** and use it in **CollectIdInfos** / **VerifyConstantPool**.
2. In **verify.cpp**, extend **Verify** to accept the option and forward it to **Verifier**.
3. Add a test that runs **ark_verifier** (or **Verify**) with the new option and checks the exit code or output.

---

## Critical pitfalls / "do not do this"

- **Do not** assume **file_** is non-null after construction; **File::Open** can fail. All public **Verify*** methods already check **file_ == nullptr** and return false with a log message.
- **Do not** run **VerifyConstantPool** or **VerifyConstantPoolContent** before **CollectIdInfos()**; **ins_method_ids_**, **ins_literal_ids_**, **ins_string_ids_**, **all_method_ids_**, and **module_literals_** must be populated first.
- **Do not** change the semantics of **constant_pool_ids_** vs **literal_ids_** vs **all_method_ids_**; **VerifyMethodId**, **VerifyLiteralId**, and **VerifyStringId** rely on their distinct roles (method id must be in constant pool, not in literal ids or string ids; etc.).
- **Do not** add checks that require compiler or runtime types; this verifier is structural only. For type/semantic verification, use the **static_core/verification** framework.
- When adding tests, ensure the test ABCs are built and **GRAPH_TEST_ABC_DIR** is set so that paths like **GRAPH_TEST_ABC_DIR "test_checksum.abc"** resolve correctly.

---

## Where to look (quick reference)

| Goal | Location |
|------|----------|
| Entry and API | **main.cpp**, **verify.h**, **verify.cpp** |
| Core logic | **verifier.h**, **verifier.cpp** |
| Build | **BUILD.gn** (ark_verifier, libarkverifier) |
| C++ tests | **tests/verify_checksum_test.cpp**, **tests/verify_constant_pool_tests.cpp**, **tests/verify_register_index_test.cpp**, **tests/utils.h**, **tests/BUILD.gn**, **tests/js/** and **tests/ts/** for ABC sources |
| Python tests | **tests/verify_262abc_files.py**, **tests/verify_es2panda_test_abc.py**, **tests/verify_sys_hap_abc.py**, **tests/version_compatibility_test.py** |
