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
    Providing implementation for the interface property
    in the form of a field with overriding implementation
    class field with override keyword
params: generic class, union of type parameters
---*/

interface I<U, V> {
  f: U | V
}

abstract class B<U, V> implements I<U, V> {
  f: U | V
  constructor(v: U | V) {
    this.f = v
  }
}

class D<U, V> extends B<U, V> {
  override f: U | V
  constructor(v1: U | V, v2: U | V) {
    super(v1)
    this.f = v2
  }
}

function get_f<U, V>(a: B<U, V>): U | V {
  return a.f
}

function set_f<U, V>(a: B<U, V>, v: U | V) {
  a.f = v
}

function main(): void {
  let a: D<int, string> = new D<int, string>(5, "abc")
  arktest.assertTrue(a.f instanceof string)
  arktest.assertEQ(a.f, "abc")
  arktest.assertEQ(get_f(a), "abc")
  set_f(a, 15)
  arktest.assertEQ(a.f, 15)
  arktest.assertEQ(get_f(a), 15)
}
