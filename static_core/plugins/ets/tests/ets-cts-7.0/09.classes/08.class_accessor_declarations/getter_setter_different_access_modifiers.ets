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
desc: >-
    Getter and setter could have different access modifiers
---*/

class A {
  _x: int;
  public get x(): int { return this._x }
  protected set x(i: int) { this._x = i }
}

class C extends A {
  foo(i: int) {
    this.x = i
  }
}

function main() {
  let i: C = new C()
  arktest.assertEQ(i.x, 0)
  i.foo(3)
  arktest.assertEQ(i.x, 3)
}
