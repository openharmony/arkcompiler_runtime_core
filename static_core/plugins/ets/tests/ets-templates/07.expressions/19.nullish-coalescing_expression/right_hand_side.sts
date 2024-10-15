/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
desc: null safety -> Null-Coalescing Operator.
assert: It checks whether the evaluation of the left-hand-side expression is equal to null and, if it is, the result of null-coalescing expression is the evaluation result of the right-hand-side expression.If the evaluation result of the left-hand-side expression is not equal to null, the result of null-coalescing expression is the evaluation result of the left-hand-side expression.
---*/

function main(): void {
  let a: Int|null = null;
  let b: Int = 4;

  let x: Int = a ?? b;

  assert x == 4;
}
