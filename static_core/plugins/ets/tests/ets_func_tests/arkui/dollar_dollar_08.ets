/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
 desc: 06 Other State Management Functionality
 name: 06.other_state_management_functionality/06.03.built-in_component_two-way_value_synchronization_'$$'_dollar_dollar_08
 tags: [negative, compile-only]
 ---*/

function increment(n: int): void {
  n++
}

function main(): void {
  let nArr: int[] = [5,5]
  increment($$nArr[0])
  assertEQ(nArr[0],  6)

  increment(nArr[1])
  assertEQ(nArr[1],  5)
}