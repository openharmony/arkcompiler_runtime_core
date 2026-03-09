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
params: generic class
---*/

class B<T> {
  f1: T
  f2: T
  constructor(v: T) {
    this.f1 = v
    this.f2 = v
  }
}

class D1<U> extends B<U> {
  override f1: U
  override f2: U
  constructor(v1: U, v2: U) {
    super(v1)
    this.f1 = v2
    this.f2 = v2
  }
}

class D2<U, V> extends B<U> {
  override f1: U
  override f2: U
  constructor(v1: U, v2: U) {
    super(v1)
    this.f1 = v2
    this.f2 = v2
  }
}

class D3<U, V> extends B<V> {
  override f1: V
  override f2: V
  constructor(v1: V, v2: V) {
    super(v1)
    this.f1 = v2
    this.f2 = v2
  }
}

function get_f1<T>(a: B<T>): T {
  return a.f1
}

function get_f2<T>(a: B<T>): T {
  return a.f2
}

function set_f1<T>(a: B<T>, v: T) {
  a.f1 = v
}

function set_f2<T>(a: B<T>, v: T) {
  a.f2 = v
}

function main(): int {
  let testSuite = new arktest.ArkTestsuite("overriding_instance_field_pos_gc");

  testSuite.addTest("check_D1", () => {
    let a: D1<int> = new D1<int>(5, 10)
    arktest.assertTrue(a.f1 instanceof int)
    arktest.assertEQ(a.f1, 10)
    arktest.assertTrue(a.f2 instanceof int)
    arktest.assertEQ(a.f2, 10)
    arktest.assertEQ(get_f1(a), 10)
    arktest.assertEQ(get_f2(a), 10)
    set_f1(a, 15)
    set_f2(a, 20)
    arktest.assertEQ(a.f1, 15)
    arktest.assertEQ(a.f2, 20)
    arktest.assertEQ(get_f1(a), 15)
    arktest.assertEQ(get_f2(a), 20)
  });

  testSuite.addTest("check_D2", () => {
    let a: D2<int, string> = new D2<int, string>(5, 10)
    arktest.assertTrue(a.f1 instanceof int)
    arktest.assertEQ(a.f1, 10)
    arktest.assertTrue(a.f2 instanceof int)
    arktest.assertEQ(a.f2, 10)
    arktest.assertEQ(get_f1(a), 10)
    arktest.assertEQ(get_f2(a), 10)
    set_f1(a, 15)
    set_f2(a, 20)
    arktest.assertEQ(a.f1, 15)
    arktest.assertEQ(a.f2, 20)
    arktest.assertEQ(get_f1(a), 15)
    arktest.assertEQ(get_f2(a), 20)
  });

  testSuite.addTest("check_D3", () => {
    let a: D3<string, int> = new D3<string, int>(5, 10)
    arktest.assertTrue(a.f1 instanceof int)
    arktest.assertEQ(a.f1, 10)
    arktest.assertTrue(a.f2 instanceof int)
    arktest.assertEQ(a.f2, 10)
    arktest.assertEQ(get_f1(a), 10)
    arktest.assertEQ(get_f2(a), 10)
    set_f1(a, 15)
    set_f2(a, 20)
    arktest.assertEQ(a.f1, 15)
    arktest.assertEQ(a.f2, 20)
    arktest.assertEQ(get_f1(a), 15)
    arktest.assertEQ(get_f2(a), 20)
  });

  return testSuite.run();
}
