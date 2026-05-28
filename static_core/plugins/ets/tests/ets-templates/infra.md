# Infrastructure: directory layout, KFL, and running the suite

Everything that lives *around* a test file: where to put it, how the ignore lists work, and how to run the suite locally.

## Directory structure

Top-level subdirectories mirror the ArkTS spec chapters (`NN.chapter_name/`, zero-padded to two digits):

```
02.lexical_elements/                # §2  — Lexical elements
03.types/                           # §3  — Types
04.names_declarations_and_scopes/   # §4
05.generics/                        # §5
06.contexts_and_conversions/        # §6
07.expressions/                     # §7
08.statements/                      # §8
09.classes/                         # §9
10.interfaces/                      # §10
11.enumerations/                    # §11
12.error_handling/                  # §12
13.modules_and_namespaces/          # §13
14.ambient_declarations/            # §14
15.semantic_rules/                  # §15
16.concurrency/                     # §16
17.experimental_features/           # §17
18.annotations/                     # §18
20.implementation_details/          # §20
23.binary_compatibility/            # §23
24.build_system/                    # §24
```

Each subdirectory nests as deep as the spec sections do, e.g.
`07.expressions/16.instanceof_expression/`,
`09.classes/06.field_declarations/05.fields_with_late_initialization/`.

## Spec chapter → CTS directory mapping

RST `N_name.rst` → test directory `0N.name/` (zero-padded). E.g. `7_expressions` → `07.expressions/`, 
`11_enums` → `11.enumerations/`, `17_experimental` → `17.experimental_features/`. Spec sources live in `../../doc/spec/`. 
Chapters 01/19/21/22 have no CTS directories (reference-only).

After a TP (Technical Preview) update that renumbers chapters, verify directory paths still match.

## KFL / ignore lists

Local notes for `ets-templates/`.

- **Ignore list / KFL** — tests that run but their results are ignored.
- Main CTS ignore list: `../test-lists/ets-cts/ets-cts-ignored.txt`.
- **Exclude list** — tests that must not run at all (hanging).
- Main CTS exclude list: `../test-lists/ets-cts/ets-cts-excluded.txt`.
- Every ignore block needs an `#ISSUE_ID` header with a short description — missing issue number is a review blocker.
- Platform-specific variants sit beside the base file: `-AMD64`, `-ARM64`, `-AMD64-AOT`, `-JIT-REPEATS`, `-ASAN`, etc.
- `declgenets2ts/.../declgen-ets2ts-cts-ignored.txt` uses `#FailKind.*` block headers instead of issue IDs.
- When a test file is renamed or deleted, update **every** ignore/exclude list that references the old path (base + 
  declgen + platform variants).
- A test in an ignore/exclude list is a **known issue, not a coverage gap**. When analyzing coverage, subtract ignored 
  tests from the "untested" count and report them separately.

## Build / testing

Full toolchain and build commands live in the parent docs (`../../../../AGENTS.md`). 
URunner entry point is `tests-u-runner-2/runner.sh`; one-liner:

```shell
./runner.sh panda-int ets-cts --force-generate --processes all --filter="*07.expressions/*"
```

Wrapper script with env setup:

```shell
export DEV_HOME=`pwd`
export PANDA_BUILD=$DEV_HOME"/build"
export ARKCOMPILER_RUNTIME_CORE_PATH=${DEV_HOME}/runtime_core
export ARKCOMPILER_ETS_FRONTEND_PATH=${DEV_HOME}/ets_frontend
export WORK_DIR=$DEV_HOME"/tmp-cts"

cd $DEV_HOME/runtime_core/static_core/tests/tests-u-runner-2
./runner.sh panda-int ets-cts --processes all --show-progress --verbose all --force-generate
```

`--filter` accepts shell globs against the *generated* test path; e.g. `--filter="*07.expressions/16.instanceof_expression/*"` narrows to one sub-chapter.
