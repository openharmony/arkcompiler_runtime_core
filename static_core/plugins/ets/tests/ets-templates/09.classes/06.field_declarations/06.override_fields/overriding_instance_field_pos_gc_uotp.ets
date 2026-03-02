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
    The term same-type in case of generic classes means that the type
    of a field in a derived class must be the same as in the base
    class instantiated with a parameter of the same type as the type
    of the field in the derived class.
params: generic class, union of type parameters
---*/

interface I<U, V> {
  f: U | V
}

abstract class B<U, V> {
  f: U | V
  constructor(v: U | V) {
    this.f = v
  }
}

class D1<U, V> extends B<U, V> {
  override f: U | V
  constructor(v1: U | V, v2: U | V) {
    super(v1)
    this.f = v2
  }
}

class D2<U, V> extends B<V, U> {
  override f: U | V
  constructor(v1: U | V, v2: U | V) {
    super(v1)
    this.f = v2
  }
}

class D3<U = int, V = string> extends B<U, V> {
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

function main(): int {
  let testSuite = new arktest.ArkTestsuite("overriding_instance_field_pos_gc_uotp");

  testSuite.addTest("check_D1", () => {
    let a: D1<int, string> = new D1<int, string>(5, "abc")
    arktest.assertTrue(a.f instanceof string)
    arktest.assertEQ(a.f, "abc")
    arktest.assertEQ(get_f(a), "abc")
    set_f(a, 15)
    arktest.assertEQ(a.f, 15)
    arktest.assertEQ(get_f(a), 15)
  });

  testSuite.addTest("check_D2", () => {
    let a: D2<int, string> = new D2<int, string>(5, "abc")
    arktest.assertTrue(a.f instanceof string)
    arktest.assertEQ(a.f, "abc")
    arktest.assertEQ(get_f(a), "abc")
    set_f(a, 15)
    arktest.assertEQ(a.f, 15)
    arktest.assertEQ(get_f(a), 15)
  });

  testSuite.addTest("check_D3", () => {
    let a: D3 = new D3(5, "abc")
    arktest.assertTrue(a.f instanceof string)
    arktest.assertEQ(a.f, "abc")
    arktest.assertEQ(get_f(a), "abc")
    set_f(a, 15)
    arktest.assertEQ(a.f, 15)
    arktest.assertEQ(get_f(a), 15)
  });

  return testSuite.run();
}
