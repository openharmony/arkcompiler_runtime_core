# AGENTS.md

Conventions for the **`ets-templates/`** CTS tree — the primary test tree for the current ArkTS 1.2 (`ets-cts` suite).

**Scope.** Tests in this tree are **language-conformance tests** for the ArkTS specification — they verify that 
the compiler and runtime behave *as the spec says*, not that internal compiler data structures are correct.
Neighbouring trees serve other concerns:

- Compiler / VM internals → `../../runtime/`, `../../compiler/`,
  `../verifier-tests-ets/`.
- Stdlib API behavior → `../ets_func_tests/`, `../ets_sdk/`.
- ANI / JS interop → `../ani/`, `../interop_js/`.

The test *runner* lives elsewhere: `static_core/tests/tests-u-runner-2` (URunner v2). New suites and workflows must use 
`tests-u-runner-2`; the legacy `static_core/tests/tests-u-runner` is kept only for `astchecker`. URunner generates 
concrete tests from templates, then runs them under a *workflow* (`panda-int`, `panda-aot`, `panda-jit`, …).

## Document map — pick by task

| If you want to …                                                                         | Read |
|------------------------------------------------------------------------------------------|---|
| Understand `.ets` / `.params.yaml` shape + Test Policies + Review advices in "✗→✓ table" | [style-guide.md](style-guide.md) |
| Copy-paste a working positive / negative / `ArkTestsuite` test                           | [examples.md](examples.md) |
| Write the license header or the YAML front matter                                        | [headers.md](headers.md) |
| Place the file in the right subdirectory, edit a KFL list, or run the suite              | [infra.md](infra.md) |


## Project metadata
- **name**: ArkTS CTS — `ets-templates/`
- **purpose**: Primary CTS test tree for current ArkTS 1.2. New tests for the default `ets-cts` suite live here.
- **primary languages**: ArkTS-Sta (`.ets`, `.d.ets` for ambient declarations), Jinja2 templates, YAML params.
- **runner**: `static_core/tests/tests-u-runner-2` (URunner v2). Legacy `tests-u-runner` is kept only for `astchecker`.

## See also (external)
- `../../doc/spec/` — ArkTS spec RST sources.
- `../../../../tests/tests-u-runner-2/agents.md` — URunner-2 internals.
---

**Maintenance.** This guide is reviewed against the latest TP after each spec release. If a rule no longer matches 
the majority pattern in `ets-templates/`, or contradicts the spec RST sources in `../../doc/spec/`, open an issue or send 
a PR amending this file (or the appropriate companion).

**Last updated:** 2026-05-27 (TP11).