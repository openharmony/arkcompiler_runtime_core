/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: >-
    Verify that a class can handle the conflict of the same-named constants between an interface and a class
    property by overriding in a class method.
tags: [compile-only, negative]
expected_error: >-
    "Semantic error ESE45073: 'CONST' is defined as a property in class 'C',
    but is overridden here in 'D' as an accessor."
---*/

interface I {
  readonly CONST: number;
}

class C {
  readonly CONST: number = 12;
}

class D extends C implements I {
  get CONST(): number {
    return 13;
  }
}
