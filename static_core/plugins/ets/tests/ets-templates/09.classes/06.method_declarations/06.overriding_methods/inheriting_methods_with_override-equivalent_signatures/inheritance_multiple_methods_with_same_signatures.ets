/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
desc: Inherit multiple methods with override-equivalent signatures.
assert: It is a compile-time error if a class C inherits a concrete method whose signature is override-equivalent with another method inherited by C.
tags: [compile-only]
---*/

class A {
  public m(): int {
    return 1;
  }
}

interface I {
  m(): int {
    return 1;
  }
}

final class C extends A {
  override m(): int {
    return 5;
  }
}
