# LLVM AOT Workflow Knowledge

This file covers optional LLVM AOT workflow for ArkTS-Sta. It focuses on purpose, eligibility, fallback, and verification boundaries rather than backend implementation details.

## Purpose

LLVM AOT uses an LLVM backend for eligible ahead-of-time compilation. It exists as an alternative AOT backend path, not as a separate language mode or a replacement for normal semantic checks.

## Eligibility And Normal AOT Difference

| Topic / stage | Meaning | Failure signal |
|---|---|---|
| Mode selection | `--paoc-mode=llvm` requests LLVM AOT | Wrong mode means no LLVM evidence exists |
| Build support | LLVM AOT must be enabled in the build | Tool rejects LLVM mode before semantic work |
| Graph eligibility | Compiler decides whether graph can lower through LLVM | Method falls back or is rejected despite normal AOT support |
| Backend lowering | LLVM backend, not normal Ark backend, lowers eligible graphs | Backend failure or unsupported instruction |
| Coverage evidence | LLVM dumps/logs or checked `RUN_LLVM` evidence plus runtime behavior | Normal AOT product evidence alone does not prove LLVM participation |
| Runtime use | Product follows normal AOT load and execution rules | LLVM success without runtime use is incomplete evidence |

## Code and Test Anchors

- LLVM AOT product path starts in `static_core/compiler/aot/aot_builder/llvm_aot_builder.*`.
- LLVM mode entry starts in `static_core/compiler/tools/paoc/paoc_llvm.*`.
- Backend integration starts in `static_core/libllvmbackend/`.
- LLVM AOT validation should include `static_core/tests/checked/llvm*.pa` or LLVM test ignore/exclude lists.

## Documentation Routing

| Scenario / Keywords | Read first |
|---|---|
| LLVM AOT mode and offline validation | `static_core/compiler/docs/compiler_doc.md` |
| paoc mode/options source | `static_core/compiler/docs/paoc.md`, `static_core/compiler/tools/paoc/paoc.yaml` |
| Backend-specific checked examples | `static_core/tests/checked/llvmaot*.pa`, `static_core/tests/checked/*llvm*.pa` |

## Boundaries

- Do not assume LLVM AOT and normal AOT differ only in code quality; eligibility and fallback are first-class behavior.
- Do not treat fallback as success when the task is LLVM AOT coverage.
- LLVM AOT claims must prove LLVM backend participation separately from ARK AOT fallback.
- Do not debug LLVM AOT failures without separating frontend graph eligibility, backend failure, and runtime loading.
- Do not change shared compiler behavior only to satisfy LLVM AOT without checking normal JIT/OSR/AOT.

## Ask Before

- allowing LLVM fallback to count as LLVM coverage or weakening fallback/rejection diagnostics.

## Common Misroutes

- If LLVM AOT produces no code, check eligibility, backend enablement, and fallback settings first.
- If LLVM AOT differs from AOT, check backend-specific lowering, fallback, and runtime load path.
- If a test passes unexpectedly, check whether it ran through fallback instead of LLVM AOT.
- If backend failure is masked, check diagnostics and failure policy.

## Modification Checklist

Before editing, confirm:

- Test evidence proves LLVM backend participation when LLVM AOT is the required path.
- Fallback expectation is explicit.
- Differences from normal AOT are classified as eligibility, backend, or runtime behavior.
- Other workflows are considered when shared compiler code is touched.

## Verification

- LLVM path is selected: logs, dumps, or tests prove LLVM AOT participation.
- Fallback is controlled: fallback enabled/disabled cases are clear.
- Runtime behavior is correct: execution matches non-LLVM AOT or expected baseline.
- Diagnostics are preserved: backend rejection/failure remains explainable.
