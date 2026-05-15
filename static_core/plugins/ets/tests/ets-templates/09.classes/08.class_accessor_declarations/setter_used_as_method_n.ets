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
desc: CTE expected if setter is used as a method
tags: [compile-only, negative]
expected_out: "Semantic error ESE0002: Type 'Int' has no call signatures."
---*/

class A {
  private _f: int = 0
  get f(): int {
    return this._f
  }
  set f(i: int) {
    this._f = i
  }
}

function main() {
  new A().f(2)
}
