/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Abstract modifier can be used for getter/setter field
---*/

abstract class A {
  protected _f: int = 0
  abstract set f(i: int)
  abstract get f(): int
}
class C extends A {
  set f(i: int) {
    this._f = i
  }
  get f(): int {
    return this._f
  }
}

function main() {
  let i: A = new C()
  i.f = 5
  arktest.assertEQ(i.f, 5)
}
