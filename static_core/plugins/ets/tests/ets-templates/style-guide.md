# CTS test style-guide

Style-guide for the shape of every test file in `ets-templates/`. Companion to [AGENTS.md](AGENTS.md); 
copy-paste templates live in [examples.md](examples.md); license + metadata fields live in [headers.md](headers.md).

## Test formats

- **Standalone test** — single `.ets` file, compiles and runs as-is.
- **Template pair** — `name.ets` (Jinja2) + `name.params.yaml` (case data). URunner expands the pair into `name_0.ets`, `name_1.ets`, … on demand.
  Removing or reordering a case in `.params.yaml` shifts every later generated index; update KFL entries in lockstep — see [infra.md](infra.md).

## Standalone-test layout

```text
[ Copyright Notice. See headers.md ]
/*---
[ YAML front matter. See headers.md ]
---*/
[ test source code ]
```

## File-level rules

- **Front matter** is the *first* `/*--- … ---*/` block; URunner reads its body as YAML.
- `expected_out` must match compiler output **character-for-character**;
  YAML folded scalars (`>-`) join lines with single spaces — use `|-` for multi-line diagnostics.
- **Negative test**: filename ends in `_n.ets`, `tags: [compile-only, negative]`, `expected_out` is a real 
  **compiler** error (`Semantic error ESExxxx: …`, `Syntax error ESYxxxx: …`, `Warning Wxxxxxx: …`)
- **Positive test**: `function main()` exercising the feature with `arktest.assertEQ` / `assertTrue` / `ArkTestsuite.addTest`.
- `desc` accurately describes the test (copy-pasting `desc:` between positive and negative variants is a recurring review finding).

## Test policies

- **Tests must be small, precise, and minimized to the exact check.**
- **No dead code,** except in tests aimed at exercising code optimization.
- **Trivial positive tests must be combined in one `.ets` file** when the cases are closely related and a failure 
  is identifiable from the `addTest` label — use `arktest.ArkTestsuite` (see [examples.md](examples.md)).
- **No compile-only positive tests in general** — every positive test must carry runtime assertions (this also 
  exercises bytecode verification and module initialization).
- **A reasonable assertion** must be present in every runtime test.
- **Do not mix positive and negative tests in one template.** Split into `name.ets` + `name_n.ets`, each with its own `.params.yaml`.
- **Do not mix several negative cases in one test** — otherwise the failing case cannot be localized quickly.
- **Use Jinja2 templates for parametrization** rather than copy-pasting near-identical `.ets` files.
- **Jinja2 nesting level ≤ 3** (for ex. `{% for %}` / `{% if %}` / `{% set %}` combined).
- **Tests are hermetic.** No filesystem assumptions outside the test's own directory, no network, no reliance on host `cwd`, locale,
  timezone, or env vars. Extra source files go through `files:` in front matter; anything else is a CTS antipattern.

## Common review blockers (✗ → ✓)

Skim this table before submitting; addressing the recurring items removes ~80% of typical review comments.

| ✗ — what trips up new authors                               | ✓ — what to do instead |
|---|---|
| Year in copyright is stale                                  | Keep it up-to-date — see *Year in copyright* in [headers.md](headers.md). |
| License inside `/*--- … ---*/`                              | License inside `/* … */`; the front matter is the *next* comment. |
| Fullwidth colon `Issue：` (U+FF1A) in commit message         | ASCII `Issue: <url>` (single space after the colon). |
| `expected_out: "TODO: fill in"`                             | Run the test once and capture the real diagnostic. If the compiler emits an unstable / placeholder message, file an issue and KFL the test until they fix it. |
| Missing `expected_out` in a `[compile-only, negative]` test | A negative test must declare the *exact* failure it triggers; without `expected_out` you cannot tell whether the file failed for the intended reason. |
| Same `desc:` copy-pasted between positive and negative      | Each test carries a `desc:` that matches its actual behavior. |
| `_neg` / `_negative` filename suffix                        | `_n` suffix (the canonical form). |
| KFL block without `#ISSUE_ID` header                        | `#34757 short reason` above every block — see [infra.md](infra.md). |
| Removing a `.params.yaml` case without re-indexing KFL      | When `_N.ets` indices shift, update every KFL entry pointing at the old `N`. |
| Hand-rolled `if (a != b) { throw new Error(...) }`          | Use the `arktest` API (`assertEQ` / `assertTrue` / `expectThrow` / …) so the failure carries location + actual-vs-expected diff. |
