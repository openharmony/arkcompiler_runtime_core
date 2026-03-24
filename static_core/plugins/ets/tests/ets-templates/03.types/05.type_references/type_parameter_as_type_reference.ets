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
desc: Type parameter as type reference
---*/

class Holder<T> {
  value: T
  constructor(v: T) {
    this.value = v
  }
  get(): T {
    return this.value
  }
}

function main(): void {
  let h: Holder<string> = new Holder<string>("hello")
  arktest.assertTrue(h.get() == "hello")
}