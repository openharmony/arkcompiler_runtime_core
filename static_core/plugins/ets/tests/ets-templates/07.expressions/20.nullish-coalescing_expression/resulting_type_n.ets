/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc:  >-
    The type of a nullish-coalescing expression is a normalized union type (see
    Union Types) formed from the following:
    - Non-nullish variant of the type of the left-hand-side expression; and
    - Type of the right-hand-side expression.

    Type T is a subtype of union type U(U1|...|Un) if for some i there is T which is a subtype of Ui
    ( Enum <: double = false || Enum <: string = false => false )
tags: [compile-only, negative]
expected_out: "#TBD, see issue #33632"
---*/

enum Enum:double { A = 1.2, B = 0.0}

function main(): void {
  let lhs: Array<Enum> = [Enum.A, Enum.B];
  let rhs: string = "aa";

  let x = lhs[1] ?? rhs;
  arktest.assertFalse(x instanceof double|string)
  let y: double|string = x //CTE
}
