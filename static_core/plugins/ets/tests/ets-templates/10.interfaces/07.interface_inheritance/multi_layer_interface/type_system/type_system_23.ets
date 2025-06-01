/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
---*/

let a: number = 10;
let b: string = 'Hello, ';

interface A {
    draw(): void;
}

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