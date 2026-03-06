/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
    Overriding abstract method.
    A compile-time error occurs if:
    - Any non-abstract subclass of A (see Abstract Classes) does not provide
    an implementation for m.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0190: B is not abstract and does not override abstract method run(): undefined in A"
---*/

abstract class A {
    abstract run(): void;
}

class B extends A {
}

function main(): void {
    let c: B = new B();
}
