/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
 desc: 05.06.01.Partial Utility Type
 name: 05.Generics/05.06.utility_types/05.06.01.partial_utility_type/partial_utility_type_0009.ets
 tags:
 ---*/

interface I {
  var_one: short
  var_two: short
}

function foo (bar: Partial<I>) {
  if (bar.var_one != undefined) {
    bar.var_one = 500
  }
}

function main(): void {
  let a: I = {var_one: 1, var_two: 2}
  foo(a.var_one)
  assertEQ( a.var_one, 500)
  assertEQ( a.var_two, 2)

  let b: I = {var_two: 3}
  foo(b.var_one)
  assertEQ( b.var_one, undefined)
  assertEQ( b.var_two, 3)
}
