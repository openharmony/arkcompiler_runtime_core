# verification (static_core) — Agent Guide

This document is the **full working guide** for **`arkcompiler/runtime_core/static_core/verification`**.

---

## What is verification (static_core)?

**verification (static_core)** is the Ark **bytecode semantic verification framework** used by the static VM / ArkTS
toolchain. It verifies **loaded** methods (runtime `Method`/`Class` objects) for:

- control-flow consistency
- resolution of referenced types/methods/fields
- typing checks and type-system consistency
- abstract interpretation over bytecode

It is **different** from **`arkcompiler/runtime_core/verifier`**, which focuses on **structural** `.abc` validation
(checksum, constant pool/literal array integrity, instruction bounds).

---

## High-level pipeline and data flow

### Entry points (public API)

Located in **`public.h` / `public.cpp`** (namespace **`ark::verifier`**):

- **`Config *NewConfig()`**, **`bool LoadConfigFile(Config *, std::string_view)`**, **`void DestroyConfig(Config *)`**
- **`Service *CreateService(config, allocator, linker, cacheFileName)`**, **`DestroyService(service, updateCacheFile)`**
- **`Status Verify(service, method, mode)`** → `OK` / `FAILED` / `UNKNOWN`

### `Verify()` flow (runtime-facing)

1. **Fast skips / cache** (in `public.cpp`):
   - skip intrinsics
   - return immediately if already verified
   - skip if verification disabled for the selected mode
   - apply debug/config-based “skip verification” rules
   - consult **`VerificationResultCache`** (if enabled)
2. **Per-language processor**:
   - pick a **`TaskProcessor`** by `SourceLang` via **`VerifierService::GetProcessor(lang)`**
   - each processor has a per-language **`TypeSystem`**
3. **Class verification**:
   - ensure the declaring class is verified first (via language plugin `CheckClass`)
4. **Per-method checks**:
   - build **`Job(service, method, methodOptions)`**
   - run **`job.DoChecks(processor->GetTypeSystem())`**
5. **Record results**:
   - cache result (if enabled)
   - set `Method::VerificationStage`

### `Job::DoChecks()` (method pipeline)

Defined in **`jobs/job.cpp`**; the order matters:

1. **ResolveIdentifiers**: resolve referenced entities used by bytecode (types, methods, fields)
2. **CheckCflow**: build **`CflowMethodInfo`** (jump targets, flags, exception handlers)
3. **UpdateTypes**: populate type-system knowledge from cached entities and signatures
4. **ABSINT**: create a **`VerificationContext`** and run **`VerifyMethod`** (abstract interpretation)

Each step can be enabled/disabled per method via **`MethodOptions`** (`RESOLVE_ID`, `CFLOW`, `TYPING`, `ABSINT`).

---

## Repository layout (where things live)

### Public interface and orchestration

- **`public.h` / `public.cpp`**: API entry points and `Verify()` implementation
- **`public_internal.h`**: internal `Config`/`Service` structs (opts + debug config + service members)
- **`verification_options.h` / `verification_options.cpp`**: `VerificationOptions` initialization / teardown
- **`verifier/verifier.cpp`**: standalone verifier executable (creates runtime, enumerates classes/methods, runs `Verify`)

### Core verification subsystems

- **Control flow**: `cflow/` (`CheckCflow`, `CflowMethodInfo`)
- **Type system**: `type/` (`TypeSystem`, `Type`, signature/type helpers)
- **Abstract interpretation**: `absint/` (`PrepareVerificationContext`, `VerifyMethod`, exec/reg contexts)
- **Values/variables**: `value/` (typed values, var bindings, variable tracking)
- **Jobs / service**: `jobs/` (`Job`, `VerifierService`, `TaskProcessor`)
- **Config**: `config/` (config parser, method options, debug breakpoints, whitelists)
- **Cache**: `cache/` (`VerificationResultCache`)
- **Plugins**: `plugins.h` + generated registration via `plugins.cpp` (includes `plugins_gen.inc`)
- **Messages**: `messages.yaml` + generated `verifier_messages*` artifacts

### Code generation inputs

- **`verification.yaml`**: compatibility checks definition used to generate helper headers
- **`messages.yaml`**: verifier message definitions (ids, levels, templates)
- **`gen/templates/`**: templates for generated headers/inc/cpp

---

## Build and test

- **CMake**: `verification/CMakeLists.txt` wires code generation and (on x86/amd64) GTest/RapidCheck tests.
  `verifier/CMakeLists.txt` builds the standalone `verifier` executable and generates its options header from
  `verifier/options.yaml`.
- **GN**: `verification.gni` lists sources used by targets that embed the verifier.

Tests live under:

- `verification/tests/` (instruction tests)
- `verification/*/tests/` (subsystem tests)
- `verification/util/tests/` (utility library tests, including property tests)

---

## How to add a new feature

### Add a new check or rule

- **Control-flow rules**: extend `cflow/` and update `CheckCflow`.
- **Type rules**: extend `type/` and/or `TypeSystem` helpers.
- **Abstract interpretation**: extend `absint/` handlers and `VerificationContext`.
- **Per-method gating**: add a new `MethodOption::CheckType` and thread it through `Job::DoChecks`.

### Add / extend configuration

- Extend `verification/config/` (handlers + parsers + default config) and update `config/README.md`.
- Prefer adding method-level options via `MethodOptionsConfig` so you can selectively enable/disable checks.

### Add a language-specific behavior

- Implement language-specific logic via the **plugin interface** (`plugins.h`): `CheckClass`, `NormalizeType`,
  access-violation checks, and `TypeSystemSetup`.
- Ensure behavior is safe when multiple languages are loaded in the same process (per-language processors exist).

---

## Critical pitfalls / “do not do this”

- **Do not** change the order of `Job::DoChecks` steps lightly: later stages rely on earlier artifacts.
- **Do not** assume a single global `TypeSystem`: each `TaskProcessor` owns a per-language `TypeSystem`.
- **Do not** bypass `VerificationResultCache` when it’s enabled (both correctness and performance regress).
- **Do not** add plugin-dependent state into shared verification objects without considering **all** languages/plugins.
- **Do not** modify `verification.yaml` or `messages.yaml` without ensuring generated files are rebuilt by CMake/GN.

---

## Where to look (quick reference)

| Goal | Location |
|------|----------|
| Public API and `Verify()` | `public.h`, `public.cpp` |
| Per-method pipeline | `jobs/job.h`, `jobs/job.cpp` |
| Per-language processors | `jobs/service.h`, `jobs/service.cpp` |
| Control-flow analysis | `cflow/cflow_check.*`, `cflow/cflow_info.*` |
| Type system | `type/type_system.*`, `type/type_type.*` |
| Abstract interpretation | `absint/absint.*`, `absint/verification_context.h` |
| Config parsing | `config/` |
| Cache | `cache/results_cache.*` |
| Standalone executable | `verifier/verifier.cpp`, `verifier/options.yaml` |

