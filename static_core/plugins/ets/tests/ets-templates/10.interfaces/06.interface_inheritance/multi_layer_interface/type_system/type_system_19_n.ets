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
desc: Modify the return value type of the interface.
tags: [compile-only, negative]
expected_out: >-
    Semantic error ESE985118: Class member sayHello with type String in B cannot implement sayHello with type undefined property from interface A
    because the implementing return type is not compatible with the interface return type.
---*/

interface A {
    a: string;
    sayHello(): void;
}

class B implements A {
    a: string;

    sayHello(): string {
        return "Hello";
    }

    constructor(a: string) {
        this.a = a;
    }
}
