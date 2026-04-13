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
    A compile-time error occurs if: Access Modifiers of the overriding field
    and of the overridden field are different.
tags: [compile-only, negative]
expected_out: "Semantic error ESE1600001: Access modifier of field 'f' in class 'D' and in the base class 'Base' must be the same."
---*/

class Base<T> {
  protected f: T
  constructor(v: T) {
    this.f = v
  }
}

class D<T> extends Base<T> {
  f: T
  constructor(v: T) {
    super(v)
    this.f = v
  }
}

function main(): void {
}
