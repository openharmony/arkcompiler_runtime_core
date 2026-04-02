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
desc: Multiple inheritance conflict detection.
tags: [compile-only, negative]
expected_out: >-
    Semantic error ESE985118: Class member draw with type void in E cannot implement draw with type String property from interface B
    because the implementing return type is not compatible with the interface return type.
---*/

let a: number = 10;
let b: string = 'Hello, ';

interface B {
    draw(): string;
}

interface C {
    draw(x: number): void;
}

interface D {
    draw(x: string): void;
}

class E implements A, B, C, D {
    draw(): void {
        a = a + 1;
    }

    draw(): string {
        return 'b';
    }

    draw(x: number): void {
        a = a + x;
    }

    draw(x: string): void {
        b = b + x;
    }
}
