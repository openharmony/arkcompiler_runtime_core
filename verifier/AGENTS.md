# verifier (runtime_core) — Agent Guide

This document is the **full working guide** for **`arkcompiler/runtime_core/verifier`** (the **runtime_core** / structural verifier).

There is a **separate** verification framework under **`arkcompiler/runtime_core/static_core/verification`** (static / ArkTS). That one performs **semantic and type verification** (control flow, type resolution, abstract interpretation). The **runtime_core verifier** does **not** do that; it focuses on **structural and referential integrity** of `.abc` files (checksum, constant pool, literal arrays, register indices, instructions). Do not confuse the two.

---

## What is the verifier (runtime_core)?

The **verifier** is a **Panda/Ark bytecode validation tool** for `.abc` files. It checks that a binary ABC file is well-formed and that its constant pool, instructions, and metadata are consistent. It does **not** perform full semantic or type verification (that is done by **static_core/verification**); it focuses on structural and referential integrity.

Main capabilities:

- **Checksum**: Verifies the file header checksum (Adler-32 over file content after the header).
- **Constant pool**: Ensures constant-pool indices are in bounds and that all method/literal/string IDs referenced in bytecode and literal arrays are valid and mutually consistent.
- **Literal arrays**: Validates each literal array (method refs, string refs, nested literal refs, tagged values, impure NaN checks). The accepted literal tag set is the `LiteralValueWidth` table in **verifier.cpp**, which **must stay a subset** of the tag set accepted by the value type switch of `LiteralDataAccessor::EnumerateLiteralVals` (**libpandafile/literal_data_accessor-inl.h**), whose `default` branch is `UNREACHABLE()`: a tag accepted by the verifier but rejected there passes verification and aborts the runtime. Hence **TAGVALUE (0x00) is rejected** and **ETS_IMPLEMENTS (0x1c) is accepted** (4 byte value). When a tag is added to or removed from `EnumerateLiteralVals`, update `LiteralValueWidth` in the same change.
- **Register index** (optional/public for tests): Ensures every instruction's vreg indices are within the method's valid register range (vregs + args), including range instructions.
- **Instructions**: Primary and secondary opcode validity, jump target validity, try/catch block structure. The last instruction of a method may not read operands beyond the code size: every instruction walk requires `bc_ins.GetNext().GetAddress() <= bc_ins_last.GetAddress()` (**IsInstructionEndInBounds**).
- **Uninitialized registers**: A may-defined dataflow analysis flags instructions reading registers which are not initialized on any path (arguments, `sta`/`mov` writes and implicitly undefined vregs count as initialized; only the initialized state is verified, not the register content).
- **Module records**: Module related instructions are only allowed in esmodule records (records with the `moduleRecordIdx` field or files with the `_ESModuleRecord` record); the content of module record literal arrays (module requests, import/export entries, module request indexes) is verified.
- **Top level await**: The `func_main_0` of a record with top level await must not contain a reachable `returnundefined` (the compiled async main returns the inner promise; a `returnundefined` on an unreachable dead code path is not rejected).
- **IC slots**: The slot allocation consistency check (**VerifyIcSlotAllocation**, enabled from bytecode version 24.0.0.0, `IC_SLOT_CHECK_VERSION`). Every one_slot/two_slot instruction (ic_slot **and** jit_ic_slot) participates in the sequential slot allocation of the frontend: **IcSlotAssignment** re-simulates the assignment of es2panda (`RegAllocator::UpdateIcSlot`/`IRNode::SetIcSlot`: slots increase in program order by 1/2 per instruction, an 8 bit only instruction above 0xff or any instruction above 0xffff gets the 0xff sentinel which disables the ic, an assignment crossing the 8 bit boundary jumps to 0x100; after any sentinel the frontend re-assigns with a two pass sort, 8 bit only instructions first — `PandaGen::ReArrangeIc`). If the operands reproduce the simulated assignment exactly (initial or rearranged layout) and the consumed total does not exceed the raw `_ESSlotNumberAnnotation` value, the method passes. Otherwise the check falls back to **VerifyIcSlotRanges** — per instruction bounds against the derived runtime slot count (`GetRuntimeSlotCount`: 0xfe/0xff are raised to 0x100, values >= 0xffff are clamped to 0x10001) and pairwise disjointness of the slot ranges — because the bytecode optimizer (es2abc `--opt-level=2`, the default) eliminates dead instructions **without recomputing the slot number annotation**, leaving gaps and an over-approximating annotation. An aliased slot (two instructions sharing a range) is always rejected: the pgo profiler and the jit bytecode info collector consume the profile type info array slots and an alias is a type confusion. A missing annotation is treated as a slot count of zero, so any slot participating instruction of the method is rejected.
- **Class literals**: The `nonStaticNum` hidden in the last item of a class literal buffer (referenced by `defineclasswithbuffer` / `callruntime.definesendableclass`) is checked against its upper and lower bounds; the upper bound subtracts all method like tags (METHOD/GETTER/SETTER/GENERATORMETHOD/ASYNCGENERATORMETHOD), which the runtime trims from the class literal tagged array.
- **Function arguments**: The `num_args` of a code item must not be less than the lower limit implied by the `_ESCallTypeAnnotation` annotation (this/new.target/func) and must equal the parameter count of the proto of the method (plus one for a non static method's implicit this): the assembler derives the code `num_args` and the proto from the same function params (**VerifyProtoArgNumber**, the shorty is walked with bounded reads in **GetProtoArgNumber**). The proto cross check only applies when the method item references a proto: from bytecode version 12.0.x (API 12) up the `proto_idx` field is reserved (`INVALID_INDEX_16`, see `BaseMethodItem::Write`). A tampered `num_args` also relaxes the register index bound (`valid_regs = num_vregs + num_args`) and is rejected. **VerifyRegisterIndex** applies the same checks (the old `ASSERT(arg_nums >= DEFAULT_ARGUMENT_NUMBER)` was a no-op in release builds).
- **Catch blocks**: A typed catch block (decoded `type_idx != INVALID_INDEX`, i.e. not a catch-all) must resolve to a valid class through the class index of its method; a catch-all block (stored as 0, decoded `INVALID_INDEX`) is always allowed.
- **Constant pool entries**: Every id (offset) stored in the constant pool (method index region) must be inside the file bounds; a 16 bit constant pool index used by an instruction which is out of bounds resolves to offset 0x0 and is rejected.
- **Method ids**: A method id referenced through the constant pool must be declared in the method items of a record of the abc file (`method_class_map_`) or be the start of a structurally valid external (foreign) method item (**IsExternalMethodId**: the `[class_idx:u16][proto_idx:u16][name_off:u32]` header fits into the file and the name resolves to a valid string item — a plain `IsExternal` range check accepts any offset inside the foreign region).
- **String ids**: A string id referenced through the constant pool must point to a valid string item (**VerifyStringItem**: [uleb utf16 length][mutf8 data][NUL] inside the file bounds, valid mutf8, matching lengths).
- **Literal ids**: A literal array id referenced through an instruction must not collide with a method or string offset. For the file versions with the literal array index table in the header (`ContainsLiteralArrayInHeader`, <= 12.0.6.0) the id must be a member of that authoritative table; newer versions collect the literal arrays of the class fields only, there the structure and tag checks of the literal array walk apply (see `CollectUtil::CollectLiteralArray`).

The **entry point** is the **ark_verifier** executable (**main.cpp**), which parses the input file path and calls **Verify(input_file)**. The core logic lives in **panda::verifier::Verifier** and can also be used via the **libarkverifier** shared library.

---

## High-level pipeline and data flow

1. **Open file**: **Verifier(filename)** opens the ABC with **panda_file::File::Open** and stores it in **file_**.

2. **Verify()** (main entry):
   - **VerifyChecksum()**: Compare **file_->GetHeader()->checksum** with Adler-32 of file content from **FILE_CONTENT_OFFSET** to end. Return false on mismatch.
   - **CollectIdInfos()**: Build ID sets used for later checks:
     - **GetConstantPoolIds()**: Fill **constant_pool_ids_** from the file's method index (constant pool region).
     - **ValidateInstructionBasics()**: Pre-validate the primary/secondary opcodes of every instruction, the constant pool index resolution of the id operands and the tag/size structure of the instruction referenced literal arrays (including nested ones). This must run before any libpandafile component (e.g. CollectUtil in GetLiteralIds) walks the bytecode, because the instruction size computation is undefined for a corrupted opcode.
     - **GetLiteralIds()** (if **include_literal_array_ids**): Fill **literal_ids_** from literal arrays in the file.
     - **CheckConstantPool(ActionType::COLLECTINFOS)**: For each non-external class and method, call **CollectIdInInstructions** to collect **ins_method_ids_**, **ins_literal_ids_**, **ins_string_ids_** from bytecode and reject module related instructions in non-esmodule records (the check runs inside the same instruction walk); collect **all_method_ids_**; for fields, **CollectModuleLiteralId** into **module_literals_** and **CollectRecordInfo** into **esmodule_record_ids_**/**tla_record_ids_**/**module_record_literal_ids_**/**lazy_import_literal_ids_**/**method_class_map_**/**class_literal_ids_**/**sendable_class_literal_ids_**; reject a direct undefined return of the func_main_0 of a TLA record (**VerifyTlaMainFunctionReturn**).
   - **VerifyConstantPool()**:
     - **CheckConstantPoolIdsBounds()**: Ensure every constant pool id is inside the file bounds.
     - **CheckConstantPoolIndex()**: Ensure constant-pool-related indices are within file bounds; every method/literal/string ID used in instructions is valid (**VerifyMethodId**, **VerifyMethodInRecord**, **VerifyLiteralId**, **VerifyStringId**).
     - **CheckConstantPool(ActionType::CHECKCONSTPOOLCONTENT)**: For each method, **CheckConstantPoolMethodContent** — verify the code size, register count and function argument lower bound (**VerifyMethodNumArgs**), instruction indices, try/catch blocks (including the catch block type_idx), instructions (register bounds, jump targets, ic slot bounds against the slot number annotation when the file version is at least **IC_SLOT_CHECK_VERSION**), and the register initialization dataflow (**VerifyMethodRegisterInitialization**).
     - **VerifyLiteralArrays()**: For each literal array, **VerifySingleLiteralArray** — validate method/string/literal refs and tagged double encoding (impure NaN check), the tag/size validity and the **nonStaticNum** bounds of class literal buffers; module record literal arrays are verified by **VerifyModuleRecord** and lazy import flag arrays by **VerifyLazyImportFlags**.

3. **Optional / test-only entry points** (not run in the default **Verify()** path but exposed for unit tests):
   - **VerifyRegisterIndex()**: For each method, iterate instructions and **CheckVRegIdx** (vreg count, range instructions, bounds).
   - **VerifyConstantPoolIndex()**: Only **CheckConstantPoolIdsBounds()** + **CheckConstantPoolIndex()**.
   - **VerifyConstantPoolContent()**: **CheckConstantPool(CHECKCONSTPOOLCONTENT)** + **VerifyLiteralArrays()**.

Key data structures:

- **file_**: **std::unique_ptr<const panda_file::File>** — the opened ABC.
- **constant_pool_ids_**: Method IDs from the constant pool region (**std::unordered_set**).
- **literal_ids_**: Literal array entity IDs (from header or CollectUtil), **std::unordered_set** (public, tests may inject the set of a previous run).
- **all_method_ids_**: All method IDs encountered when iterating classes/methods (**std::unordered_set**).
- **ins_method_ids_**, **ins_literal_ids_**, **ins_string_ids_**: IDs referenced in bytecode (from COLLECTINFOS).
- **module_literals_**: Literal IDs used as module field values.
- **method_class_map_**: Method offset to the offset of the record which declares the method.
- **class_literal_ids_** / **sendable_class_literal_ids_**: Literal arrays referenced by defineclasswithbuffer / callruntime.definesendableclass.
- **module_record_literal_ids_** / **lazy_import_literal_ids_**: Literal arrays referenced by the moduleRecordIdx / moduleRequestPhaseIdx fields.
- **esmodule_record_ids_** / **tla_record_ids_**: Records with the moduleRecordIdx field / records with a non-zero hasTopLevelAwait field.
- **literal_walk_cache_**: Bounded cache of the literal array walk results of **ValidateLiteralStructure**, reused by **VerifySingleLiteralArray** instead of walking the buffer twice.
- **instruction_index_map_**: Precomputed instruction pointer → index for jump/try verification.
- **MethodInfos** (private): Bundles bytecode pointers, method accessor and out-parameters for **VerifyMethodInstructions** (register count etc.), the ic slot state (**IcSlotState**, the raw slot number annotation) and the collected ic slot instructions (**std::vector<IcSlotInsnInfo>**).
- **IcSlotAssignment** (private, verifier.cpp): Simulates the sequential ic slot allocation of the es2panda frontend (initial single pass and the two pass overflow sort) for **VerifyIcSlotAllocation**.
- **RegisterDefinitionDataflow** (private, verifier.cpp): The word parallel may-defined dataflow engine of the register initialization analysis; the method size is bounded by **MAX_REGISTER_ANALYSIS_COMPLEXITY** (state memory, insns × regs) and **MAX_REGISTER_ANALYSIS_TIME_COMPLEXITY** (worst case dataflow work, insns × regs²), larger methods skip the analysis.

---

## Repository layout (where things live)

### Entry and public API

- **main.cpp**: **ark_verifier** executable — **PandArgParser** (help, input_file), then **Verify(input_file)**; exit 0 on success, 1 on failure.
- **verify.h** / **verify.cpp**: **Verify(const std::string &input_file)** — constructs **Verifier**, returns **Verifier::Verify()**. Thin wrapper for the library/executable API.
- **verifier.h**: **panda::verifier::Verifier** — constructor taking filename; **Verify()**, **CollectIdInfos()**, **VerifyChecksum()**, **VerifyConstantPool()**, **VerifyRegisterIndex()**, **VerifyConstantPoolIndex()**, **VerifyConstantPoolContent()**; constants (TAG_BITS_SIZE, MAX_REGISTER_INDEX, etc.); optional **include_literal_array_ids**, **literal_ids_**, **inner_literal_map_**, **inner_method_map_**. Private helpers and implementation types (**MethodInfos**, **IcSlotState**, **LiteralArrayWalkResult**, **RegisterDefinitionDataflow**) for ID collection, constant-pool checks, literal-array checks, register-index checks, jump/try-catch verification.
- **verifier.cpp**: Implements all Verifier methods — checksum, GetConstantPoolIds/GetLiteralIds, CheckConstantPool (COLLECTINFOS / CHECKCONSTPOOLCONTENT), CollectIdInInstructions, VerifyLiteralArrays, VerifySingleLiteralArray, VerifyMethodId/VerifyLiteralId/VerifyStringId, VerifyRegisterIndex, CheckVRegIdx, GetVRegCount, jump and try-block verification, slot number from annotation, etc.

### Tests

- **tests/BUILD.gn**: Defines **VerifierTest** (host_unittest_action) — C++ sources: **utils.cpp**, **verify_checksum_test.cpp**, **verify_constant_pool_tests.cpp**, **verify_new_checks_test.cpp**, **verify_register_index_test.cpp**. Deps: **libarkverifier**, libpandabase, libpandafile. Generates ABCs from **tests/js/** and **tests/ts/** via **es2abc_gen_abc** (module fixtures are compiled with `--module`). **GRAPH_TEST_ABC_DIR** points to the directory containing the generated .abc files.
- **tests/utils.h** / **tests/utils.cpp**: Test helpers.
- **tests/config.py**: Shared config for Python tests.
- **tests/verify_262abc_files.py**, **tests/verify_es2panda_test_abc.py**, **tests/verify_sys_hap_abc.py**, **tests/version_compatibility_test.py**: Python-based verification tests.
- **tests/verify_handcrafted_abcs.py** / **tests/abc_builder.py**: End-to-end tests using hand-crafted abc files (run `python3 tests/verify_handcrafted_abcs.py <path-to-ark_verifier>`), no es2abc needed — the minimal abc files are built by **AbcBuilder**.
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

1. In **CheckConstantPoolMethodContent** or **VerifySingleLiteralArray** (and related **VerifyMethodIdInLiteralArray**, **VerifyStringItem**, **VerifyLiteralIdInLiteralArray**), add the new rule. Reuse **constant_pool_ids_**, **literal_ids_**, **all_method_ids_**, **ins_*_ids_**, **module_literals_** so that cross-references stay consistent.
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
| C++ tests | **tests/verify_checksum_test.cpp**, **tests/verify_constant_pool_tests.cpp**, **tests/verify_new_checks_test.cpp**, **tests/verify_register_index_test.cpp**, **tests/utils.h**, **tests/BUILD.gn**, **tests/js/** and **tests/ts/** for ABC sources |
| Python tests | **tests/verify_262abc_files.py**, **tests/verify_es2panda_test_abc.py**, **tests/verify_sys_hap_abc.py**, **tests/version_compatibility_test.py**, **tests/verify_handcrafted_abcs.py** (hand-crafted abcs via **tests/abc_builder.py**) |
