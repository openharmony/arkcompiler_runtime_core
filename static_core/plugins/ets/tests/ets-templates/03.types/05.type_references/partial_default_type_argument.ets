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
desc: Partial default type argument in class type reference
---*/

class Pair<T, U = string> {
  first: T
  second: U

  constructor(first: T, second: U) {
    this.first = first
    this.second = second
  }
}

function main(): void {
  let x: Pair<number> = new Pair<number, string>(10, 'ten')
  let y: Pair<boolean, number> = new Pair<boolean, number>(true, 1)

  arktest.assertEQ(x.first, 10)
  arktest.assertEQ(x.second, 'ten')
  arktest.assertEQ(y.second, 1)
}
