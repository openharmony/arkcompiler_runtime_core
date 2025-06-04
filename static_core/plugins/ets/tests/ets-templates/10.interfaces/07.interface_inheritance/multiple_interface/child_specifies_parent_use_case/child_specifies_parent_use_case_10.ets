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
desc: Verify that a child interface resolves method conflicts when parent interfaces have methods with different parameter orders.
---*/

interface Parent1 {
    greet(name: string, age: number): string;
}

interface Parent2 {
    greet(age: number, name: string): string;
}

class Child1Impl implements Parent1, Parent2 {
    greet(name: string, age: number): string {
        return `Hello ${name}, ${age} years old from Parent1`;
    }

    greet(age: number, name: string): string {
        return `Hello ${name}, ${age} years old from Parent2`;
    }
}

function main(): void {
    const child = new Child1Impl();
    assertEQ(child.greet("David", 28), "Hello David, 28 years old from Parent1");
    assertEQ(child.greet(28, "David"), "Hello David, 28 years old from Parent2");
}