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
params: generic class, different covariant subtype values
---*/

class A<T> {
  f: T
  constructor(v: T) {
    this.f = v
  }
}

class B<U> extends A<U> {
  f: U
  constructor(v1: U, v2: U) {
    super(v1)
    this.f = v2
  }
}

class C<U> extends A<U> {
  f: U
  constructor(v1: U, v2: U) {
    super(v1)
    this.f = v2
  }
}

class Base<V> {
  f: A<V>
  constructor(v1: V, v2: V) {
    this.f = new B<V>(v1, v2)
  }
}

class D<T> extends Base<T> {
  f: A<T>
  constructor(v1: T, v2: T) {
    super(v1, v2)
    this.f = new C<T>(v1, v2)
  }
}

function get_f<T>(a: Base<T>): A<T> {
  return a.f
}

function main(): void {
  let a: D<int> = new D<int>(5, 10)
  let b: Base<int> = new Base<int>(3, 4)
  arktest.assertTrue(a.f instanceof C)
  arktest.assertEQ(a.f.f, 10)
  arktest.assertTrue(get_f(a) instanceof C)
  arktest.assertEQ(get_f(a).f, 10)
  arktest.assertTrue(b.f instanceof B)
  arktest.assertEQ(b.f.f, 4)
  arktest.assertTrue(get_f(b) instanceof B)
  arktest.assertEQ(get_f(b).f, 4)
}
