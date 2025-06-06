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
desc: Verify multi-level inheritance of interfaces, abstract classes, 
      and concrete classes to ensure correct method overrides and invocations.
---*/

interface A {
    a: number;
}

abstract class B implements A {
    a: number = 5;
    abstract getB(): number;
}

class C extends B {
    getB(): number {
        return this.a;
    }
}

function main(): void {
    const c = new C();
    assertEQ(c.getB(), 5);
}