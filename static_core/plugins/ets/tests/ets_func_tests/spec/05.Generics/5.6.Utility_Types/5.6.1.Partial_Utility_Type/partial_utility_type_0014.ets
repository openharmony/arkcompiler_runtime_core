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
 name: 05.Generics/05.06.utility_types/05.06.01.partial_utility_type/partial_utility_type_0014.ets
 tags: []
 ---*/

interface I { value: int }

function f(i: Partial<I>) {
  const v = i.value ?? 0
  return v
}

function main(): void {
  assertEQ(f({}), 0)
  assertEQ(f({value: 2}), 2)
}