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
desc: Verify multi-level interfaces and class inheritance to ensure correct access to constants and methods.
---*/

interface A {
    a: number;
}

abstract class B implements A {
    a: number = 11;
    abstract getB(): number;
}

class C extends B {
    getB(): number {
        return this.a;
    }
}

class D extends C {
    getD(): number {
        return this.a + 1;
    }
}

function main(): void {
    const d = new D();
    assertEQ(d.getB(), 11);
    assertEQ(d.getD(), 12);
}