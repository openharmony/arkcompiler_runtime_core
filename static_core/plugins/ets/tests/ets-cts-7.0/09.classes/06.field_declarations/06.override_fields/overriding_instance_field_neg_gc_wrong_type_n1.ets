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
tags: [compile-only, negative]
expected_out: "Semantic error ESE1049305: An overridden field 'f' must be initialized explicitly, either by using an initializer or in a constructor."
---*/

class B<T> {
  f: T
  constructor(v: T) {
    this.f = v
  }
}

class D<U, V> extends B<U> {
  f: V
  constructor(v: U) {
    super(v)
  }
}

function main(): void {
}
