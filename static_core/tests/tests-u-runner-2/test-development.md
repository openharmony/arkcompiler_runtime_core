# Test Development Guide

This document describes how to create and annotate ETS test files for the URunner test framework.
It covers the YAML front-matter markup syntax, supported metadata fields, and conventions for expected output validation.

---

## Table of Contents

1. [Test Metadata Reference](#test-metadata-reference)
2. [Metadata Fields](#metadata-fields)
3. [`negative_steps` Metadata Field](#negative_steps-metadata-field)
4. [`files` Metadata Field](#files-metadata-field)
5. [`test_suffix` and `file_name` Metadata Fields](#test_suffix-and-file_name-metadata-fields)
6. [Expected Output/Error Check](#expected-outputerror-check)
7. [Supported Tags Reference](#supported-tags-reference)
8. [Failure Check for Ignored Tests](#failure-check-for-ignored-tests)
9. [ETS-CTS Generation by Filter](#ets-cts-generation-by-filter)
10. [Proposed Additional Topics](#proposed-additional-topics)

---

## Test Metadata Reference

ETS test files can include a YAML front-matter block (`/*--- ... ---*/`) with metadata fields:

```
/*---
desc: Test description.
tags: [negative]
files: ['./helper.ets']
negative_steps: [verify-step, runtime-step]
expected_error:
    verify-step: |-
        E/verifier: Error: method void ETSGLOBAL::main() failed to verify
    runtime-step:
    - "E/verifier: Cannot link class: Cannot find class Lsome/class;"
---*/
```

## Metadata Fields

| Field | Description                                                                                                |
|---|------------------------------------------------------------------------------------------------------------|
| `desc` | Human-readable description of what the test checks                                                         |
| `tags` | Test tags. Common values: `negative` (test is expected to fail), `compile-only`, `no-warmup`, `not-a-test` |
| `negative_steps` | List of step names expected to exit with a non-zero return code                                            |
| `files` | Additional files required by the test                                                                      |
| `expected_error` | Expected stderr content per step (matched as regex or substring)                                           |
| `expected_out` | Expected stdout content per step                                                                           |
| `file_name` | Override the generated test file base name. Only for `ets-cts*` test suites.                               |
| `test_suffix` | Override the generated test file suffix. Only for `ets-cts*` test suites.                                                                    |

---

## `negative_steps` Metadata Field

The `negative_steps` field defines a list of step names that are expected to finish with a non-zero return code.

Example:

```yaml
tags: [negative]
negative_steps:
  - step_name1
  - step_name2
```

If `negative_steps` is not specified, the following behavior is used by default:

- `tags: [compile-only, negative]`: the compiler step is expected to fail (the first compiler step if there are multiple).
- `tags: [negative]`: the runtime step is expected to fail (the first runtime step if there are multiple).

---

## `files` Metadata Field

The `files` metadata field is a list of additional files required by the test. Usually, they are imported files that requires to successful run.
Format: `['/path/to/file1.ets', '/path/to/file2.ets']` where paths are relative to the main test file.

For example: the test file `main.ets` imports a file `helper.ets`, which is located near the `main.ets` file:

```javascript
// markup for the main.ets file
/*---
    files: ['./helper.ets']
---*/
```

The dependent file `helper.ets` should be marked as `compile-only` and `not-a-test` tags:
```javascript
// markup for the helper.ets file
/*---
    tags: [compile-only, not-a-test]
---*/
```

**Note**: It makes no sense to mark dependent files as `negative`. In such case the test execution can stop earlier than expected.

---

## `test_suffix` and `file_name` Metadata Fields

Test template metadata was extended with two fields: `file_name` and `test_suffix`.
These fields allow overriding the generated test file base name and suffix.
> Only for `ets-cts*` test suites.

| Field | Effect |
|---|---|
| `file_name` | Overrides the base name of the generated test file |
| `test_suffix` | Overrides the suffix appended to the generated test file name |

---

## Expected Output/Error Check

Expected output/error validation supports expected `stdout`/`stderr` from either:

- sidecar files (`.expected` / `.expected.err`)
- test metadata (`expected_out` / `expected_error`)

Metadata expectations can be scoped to a specific workflow step (by step name or step kind),
with a default fallback to the compiler step when no step is explicitly specified.

**Note**: sidecar files are deprecated and no new file should be created. For new tests use metadata.

Example test markup:

```text
/*---
desc: Test File.
tags: [negative]
files: ['./renamed_namespace.libr.ets']
negative_steps: [verify-new-libs, runtime-with-new-libs.ark]
expected_error:
    verify-new-libs: |-
        E/verifier: Error: method void renamed_namespace.ETSGLOBAL::main() failed to verify
    runtime-with-new-libs.ark:
    - "E/verifier: Cannot link class: Cannot find class Lrenamed_namespace/libr/a;"
---*/
```

---

## Supported Tags Reference
Following tags are supported:
 - `compile-only` - the test is supposed to run only compiler steps.
 - `negative` - the test is expected failed. Better to use it along with expected_error field for precise error checking.
 - `not-a-test` - the file is not considered as a test. Used to mark up dependent files.
 - `no-warmup` - the test is not changed for JIT with repeats. Otherwise, the main function is wrapped with a loop.

---

## Failure Check for Ignored Tests

Expected-failure matching was added for tests from ignored lists.

An ignored-list entry can include a preceding comment with a failure marker, for example:

```text
#12345 issue description @@Failure: <failure_regex>@@
```

During execution, the runner compares the actual failure output with this pattern.
If the pattern no longer matches, the test is reclassified as a `NEW FAILURE` instead of a known ignored failure.

---

## ETS-CTS Generation by Filter

ETS-CTS template generation is optimized. Instead of always generating the full test set, generation is now scoped to the requested subset:

- `--filter`: generates only tests that match the filter.
- `--test-file`: generates only the specified test file.

This reduces unnecessary generation work and improves overall runner performance.

---
