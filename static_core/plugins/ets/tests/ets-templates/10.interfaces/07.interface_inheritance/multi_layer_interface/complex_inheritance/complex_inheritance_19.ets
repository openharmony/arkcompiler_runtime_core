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
desc: The test class inherits constants from both the abstract class and the implementation interface.
---*/

class A {
    greet(): string {
        return "Hello from A";
    }
}

class C extends A {
    greet(): string {
        return "Hello from C";
    }
}

interface B {
    greet(): string;
}

interface D {
    readonly CONSTANT: string;
}

class E extends C implements B, D {
    readonly CONSTANT = "Constant from E";
}

function main(): void {
    const e = new E();
    assertEQ(e.greet(), "Hello from C");
    assertEQ(e.CONSTANT, "Constant from E");
}