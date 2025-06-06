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
desc: The combination of test method overriding and constant access.
---*/

interface G {
    readonly CONSTANT: string;
    greet(): string {
        return "Constant from interface";
    }
}

class A implements G{
    static readonly CONSTANT = "Constant from A";
    readonly CONSTANT = "Constant from A";
    greet(): string {
        return "Hello from A";
    }
}

class B extends A {
    static readonly CONSTANT = "Constant from B";
    greet(): string {
        return "Hello from B";
    }
}

function main(): void {
    const b = new B();
    assertEQ(b.greet(), "Hello from B");
    assertEQ(b.CONSTANT, "Constant from A");
    assertEQ(B.CONSTANT, "Constant from B");
}