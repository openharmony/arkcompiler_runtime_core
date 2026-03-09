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
params: generic class, getting overridden field through array element
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
  f1: U
  f2: U
  constructor(v1: U, v2: U) {
    super(v1)
    this.f1 = v2
    this.f2 = v2
  }
}

class D2<U, V> extends B<U> {
  f1: U
  f2: U
  constructor(v1: U, v2: U) {
    super(v1)
    this.f1 = v2
    this.f2 = v2
  }
}

class D3<U, V> extends B<V> {
  f1: V
  f2: V
  constructor(v1: V, v2: V) {
    super(v1)
    this.f1 = v2
    this.f2 = v2
  }
}

function main(): int {
  let testSuite = new arktest.ArkTestsuite("overriding_instance_field_pos_gc_goftae");

  testSuite.addTest("check_D1", () => {
    let a: D1<int>[] = [new D1<int>(5, 10)]
    arktest.assertTrue(a[0].f1 instanceof int)
    arktest.assertEQ(a[0].f1, 10)
    arktest.assertTrue(a[0].f2 instanceof int)
    arktest.assertEQ(a[0].f2, 10)
    a[0].f1 = 15
    a[0].f2 = 20
    arktest.assertEQ(a[0].f1, 15)
    arktest.assertEQ(a[0].f2, 20)
  });

  testSuite.addTest("check_D2", () => {
    let a: D2<int, string>[] = [new D2<int, string>(5, 10)]
    arktest.assertTrue(a[0].f1 instanceof int)
    arktest.assertEQ(a[0].f1, 10)
    arktest.assertTrue(a[0].f2 instanceof int)
    arktest.assertEQ(a[0].f2, 10)
    a[0].f1 = 15
    a[0].f2 = 20
    arktest.assertEQ(a[0].f1, 15)
    arktest.assertEQ(a[0].f2, 20)
  });

  testSuite.addTest("check_D3", () => {
    let a: D3<string, int>[] = [new D3<string, int>(5, 10)]
    arktest.assertTrue(a[0].f1 instanceof int)
    arktest.assertEQ(a[0].f1, 10)
    arktest.assertTrue(a[0].f2 instanceof int)
    arktest.assertEQ(a[0].f2, 10)
    a[0].f1 = 15
    a[0].f2 = 20
    arktest.assertEQ(a[0].f1, 15)
    arktest.assertEQ(a[0].f2, 20)
  });

  return testSuite.run();
}
