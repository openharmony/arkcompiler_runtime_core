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

class B extends A {
    greet(): string {
        return "Hello from B";
    }
}

interface C {
    greet(): string;
    readonly CONSTANT: string;
}

class D extends B implements C {
    readonly CONSTANT = "Constant from D";
}

function main(): void {
    const d = new D();
    assertEQ(d.greet(), "Hello from B");
    assertEQ(d.CONSTANT, "Constant from D");
}