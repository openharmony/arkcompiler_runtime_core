/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Static method declaration with parameter of type declared in body of method class.
assert: It is a compile-time error to use the name of a parameter of any surrounding declaration in the header or body of a class. 
tags: [compile-only, negative]
---*/

class A {
  inner class B {

  }

  static foo(b: B): void;
}
