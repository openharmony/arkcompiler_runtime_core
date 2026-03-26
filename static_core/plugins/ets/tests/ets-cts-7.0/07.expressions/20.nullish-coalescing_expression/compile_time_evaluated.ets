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
desc: |-
    If an nullish-coalescing expression is known at compile time to always evaluate to the left-hand-side
    expression or to the right-hand-side expression at runtime, then a compile-time warning is issued.
expected_out: ["warning: left-hand-side expression is always used", "warning: right-hand-side expression is always used", "warning: right-hand-side expression is always used", "warning: right-hand-side expression is always used", "warning: right-hand-side expression is always used"]
---*/


function main(): void {
  let a: number = 1
  let b = undefined
  let c = null

  arktest.assertEQ(a ?? 2, 1, "lhs is always used")
  arktest.assertEQ(b ?? 3, 3, "undefined: rhs is always used")
  arktest.assertEQ(c ?? 4, 4, "null: rhs is always used")
  arktest.assertEQ(undefined ?? 5, 5, "undefined 2: rhs is always used")
  arktest.assertEQ(null ?? 6, 6, "null 2: rhs is always used")
}
