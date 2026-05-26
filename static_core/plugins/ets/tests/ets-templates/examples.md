# Your first CTS test (full working examples)

Copy one of the three minimal templates below and adjust the inner code.
Conventions live in [style-guide.md](style-guide.md) + [headers.md](headers.md); placement under `ets-templates/` — [infra.md](infra.md).

## Minimal positive test

```typescript
/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

/*---
desc: §7.32 Assignment — `+=` widens `int` to `double`.
---*/

function main(): void {
    // Arrange
    let x: double = 1.0
    // Act
    x += 2 as int
    // Assert
    arktest.assertDoubleEQ(x, 3.0, 1e-9, "double += int widens to double")
}
```

Filename: `name.ets`, placed under `07.expressions/32.assignment/`.

## Minimal negative test

```typescript
/* … same Apache 2.0 header as above … */

/*---
desc: §9.6 — assigning to a `readonly` field is a compile-time error.
tags: [compile-only, negative]
expected_out: |-
    Semantic error ESE4002: Cannot assign to a readonly field x
---*/

class C {
    readonly x: int = 0
}

function main(): void {
    let c = new C()
    c.x = 1                           // CTE per §9.6
}
```

Filename: `name_n.ets`, same chapter directory. The `ESExxxx` code in `expected_out:` must come from a real compiler run — see
[*Common review blockers*](style-guide.md) for the `TODO:` placeholder anti-pattern.

## Minimal multi-case positive test using `ArkTestsuite`

Use this when several closely-related cases would otherwise become copy-pasted `function main()` blocks. See *Test Policies* in [style-guide.md](style-guide.md).

```typescript
/* … same Apache 2.0 header as in the positive example above … */

/*---
desc: §6.4.1 Widening Numeric Conversions — int → double on assignment.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("int widening to numeric types")

    suite.addTest("int → long", () => {
        let v: long = (1 as int)
        arktest.assertEQ(v, 1 as long)
    })
    suite.addTest("int → double", () => {
        let v: double = (1 as int)
        arktest.assertDoubleEQ(v, 1.0, 1e-9)
    })

    return suite.run()
}
```

- `function main(): int` returning `suite.run()` (runner reads exit code).
- `addTest` labels must be unique within the suite and descriptive — never bare numbers.
- In templates derive the suite name from case data (`new arktest.ArkTestsuite("{{c.from}}_to_{{c.to}}")`) rather than a constant.
