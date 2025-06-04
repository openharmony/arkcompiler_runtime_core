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
desc: Verify the access to constants under complex inheritance.
---*/

interface G {
    value: number;

    getValue(): number {
        return this.value;
    }

    greet(): string {
        return "Hello G";
    }
}

interface F extends G {

}

interface E extends F {
    greet(): string {
        return "Hello E";
    }
}

class A implements E {
    static readonly COMMON_VALUE: number = 100;
    value: number;

    constructor(value: number) {
        this.value = value;
    }

    getValue(): number {
        return this.value;
    }

    greet(): string {
        return "Hello";
    }
}

class B extends A {
    greet(): string {
        return `Hello, ${this.value}`;
    }

    constructor(value: number) {
        super(value);
    }
}

class C extends A {
    constructor(value: number) {
        super(value);
    }

    greet(): string {
        return `Hello, ${this.value}`;
    }

    getValue(): number {
        return this.value + 200;
    }
}

function main(): void {
    const c = new C(200);
    assertEQ(C.COMMON_VALUE, 100);
    assertEQ(c.value, 200);
    assertEQ(c.getValue(), 400);
    assertEQ(c.greet(), "Hello, 200")
    
    const b = new B(300);
    assertEQ(B.COMMON_VALUE, 100);
    assertEQ(b.value, 300);
    assertEQ(b.getValue(), 300);
    assertEQ(b.greet(), "Hello, 300")
}