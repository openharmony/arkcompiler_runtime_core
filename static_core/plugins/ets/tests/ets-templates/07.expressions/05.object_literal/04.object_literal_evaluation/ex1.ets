/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
    The execution of the object literal completes abruptly if so does the execution of a name-value pair.
---*/

class A {
    name: string|null = null
    age: number|null = null
}

function main(): int {
  let t: number[] = [1, 2, 3, 4, 5]
  let a: A|null = null;
  try {
      a = { name: "Bob", age: t[5] } as A
  } catch (e: RangeError) {
      assertTrue( a == null );
      return 0;
  }
  return 1;
}
