/*---
Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
desc: Access to constant class variable from inner class.
assert: Local classes have access to instance members of the enclosing class and local variables if such are declared constant (i.e., a variable or parameter whose value remains unchanged after initialization).
tags: [compile-only]
---*/

class A {
  public const x: int = 1;

  inner class B {
    public bar(): void {
      let y: int = x + 1;
    }
  }
}
