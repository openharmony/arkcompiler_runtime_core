# File headers: copyright + test metadata

Both blocks live at the **top** of every test file, in the order shown: 
license first (regular comment), front matter second (`/*--- … ---*/`).
Mixing them up triggers the URunner trap.

## Copyright notice — `.ets` file

```text
/*
 * Copyright (c) XXXX-YYYY Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
```

## Copyright notice — `.params.yaml` file

Identical Apache 2.0 text, each line prefixed with `# `:

```text
# Copyright (c) XXXX-YYYY Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
```

> **CRITICAL — URunner front-matter trap.** The Copyright Notice MUST use `/* … */`, **never** `/*--- … ---*/`.
> URunner parses every `/*--- ... ---*/` block as YAML front matter (`desc:` / `tags:` / `expected_out:`; 
> license accidentally wrapped in those brackets fails test generation with "Splitting tests fails: Invalid data format".
> Only the real front matter belongs in /*--- … ---*/.

## Year in copyright

Each notice carries a **single year XXXX** (year of creation) or a **range XXXX-YYYY** (XXXX = creation, YYYY = last edit). If
`YYYY == XXXX`, drop the range and use the single year.

Concrete examples (current year is 2026):

- New file created today:
```text
Copyright (c) 2026 Huawei Device Co., Ltd.
```

- Pre-existing file modified today: keep the original start year, bump the end year to 2026 — e.g.
```text
Copyright (c) 2021-2026 Huawei Device Co., Ltd.
```
- Format details: plain hyphen with no spaces around it (`2021-2026`, **not** `2021 - 2026`).

## Test metadata (front matter)

The `/*--- … ---*/` block immediately after the license is parsed by
URunner as a YAML document. Required fields:

- `desc:` — text description of the spec assertion(s) the test covers. Single line, or use `>-` folded scalar for 
  multi-line. Must accurately describe the test's actual behavior (copy-paste between positive and negative 
  variants is a recurring review finding).
- `tags:` — `[]` or omitted for a plain positive test;
  `[compile-only, negative]` for a CTE-rejecting test (note the order and the single space after the comma); 
  `[compile-only]` for no-runtime cases like warning checks; `[not-a-test, compile-only]` for helper / `.d.ets` companions.
- `expected_out:` — required for `[compile-only, negative]` and `[compile-only]`. Use the literal block scalar `|-`
  to match character-for-character. Real compiler diagnostic only — see the *Common review blockers* table in [style-guide.md](style-guide.md).

Other metadata fields you may encounter:

- `files:` — companion `.ets` / `.d.ets` files that must be compiled alongside this test (multi-file / ambient-declaration scenarios).
- `test_suffix:` — override the default `_N` index in generated names (e.g. `test_suffix: "{{type}}"` produces `name_int.ets`, `name_string.ets`).

For the full list of metadata fields and their semantics, see `../../../../tests/tests-u-runner-2/agents.md`.
