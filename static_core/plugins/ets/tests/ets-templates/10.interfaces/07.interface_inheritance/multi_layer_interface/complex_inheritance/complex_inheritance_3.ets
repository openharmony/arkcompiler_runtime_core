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
desc: Verify multi-level inheritance of interfaces, classes and abstract classes to ensure correct access to constants and methods.
---*/

interface A {
    a: number;
}

class B implements A {
    a: number = 6;
}

abstract class C extends B {
    abstract getC(): number;
}

class D extends C {
    getC(): number {
        return this.a;
    }
}

function main(): void {
    const d = new D();
    assertEQ(d.getC(), 6);
}