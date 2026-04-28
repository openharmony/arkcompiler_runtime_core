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

## ETS-CTS Generation by Filter

ETS-CTS template generation is optimized. Instead of always generating the full test set, generation is now scoped to the requested subset:

- `--filter`: generates only tests that match the filter.
- `--test-file`: generates only the specified test file.

This reduces unnecessary generation work and improves overall runner performance.

## `test_suffix` and `file_name` Metadata Fields

Test template metadata was extended with two fields: `file_name` and `test_suffix`.
These fields allow overriding the generated test file base name and suffix.

## Failure Check for Ignored Tests

Expected-failure matching was added for tests from ignored lists.

An ignored-list entry can include a preceding comment with a failure marker, for example:

```text
#12345 issue description @@Failure: <failure_regex>@@
```

During execution, the runner compares the actual failure output with this pattern.
If the pattern no longer matches, the test is reclassified as a `NEW FAILURE` instead of a known ignored failure.

## Expected Output/Error Check

Expected output/error validation supports expected `stdout`/`stderr` from either:

- sidecar files (`.expected` / `.expected.err`)
- test metadata (`expected_out` / `expected_error`)

Metadata expectations can be scoped to a specific workflow step (by step name or step kind),
with a default fallback to the compiler step when no step is explicitly specified.

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
