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
 desc: Generic with default type argument
 ---*/

class BoxT<T = number> {
  value: T
  constructor(v: T) {
    this.value = v
  }
}

function main(): void {
  let x: BoxT = new BoxT(5)
  let y: BoxT<string> = new BoxT<string>("abc")

  arktest.assertTrue(x.value == 5)
  arktest.assertTrue(y.value == "abc")
}