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
desc: Inheritance of private methods.
assert: The interface inherits, from the interfaces it extends, all members of those interfaces, except for private methods.
tags: [compile-only, negative]
---*/

interface A {
  private m(): int { return 5; }
}

class B implements A {
  public static bar(): int {
    return m();
  }
}
