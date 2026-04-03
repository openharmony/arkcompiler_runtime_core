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
desc: >-
    If the result of the evaluation of expr in expr! is not equal to null
    or undefined, then the result of ensure-not-nullish expression is
    the outcome of the evaluation of expr, otherwise NullPointerError is thrown.
---*/

function foo(arg: Object) : int {
  return 1;
}

function foo(arg: Object|null) : int {
  return 2;
}

let a: Object|null = new Object();
let b: Object|null = null;

function main(): void {
  arktest.assertEQ(foo(a), 2)
  arktest.assertEQ(foo(a!), 1)
  arktest.assertEQ(foo(b), 2)
  arktest.expectThrow(
    () => { foo(b!) },
    (e: Error) => e instanceof NullPointerError
  )
}
