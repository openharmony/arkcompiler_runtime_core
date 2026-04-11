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
desc: Nested alias replacement through array and generic reference
---*/

class BoxT<T> {
  value: T
  constructor(v: T) {
    this.value = v
  }
}

type Num = number
type NumBox = BoxT<Num>
type NumBoxList = NumBox[]

function main(): void {
  let xs: NumBoxList = [new BoxT<number>(1), new BoxT<number>(2)]
  arktest.assertTrue(xs.length == 2)
  arktest.assertTrue(xs[0].value + xs[1].value == 3)
}
