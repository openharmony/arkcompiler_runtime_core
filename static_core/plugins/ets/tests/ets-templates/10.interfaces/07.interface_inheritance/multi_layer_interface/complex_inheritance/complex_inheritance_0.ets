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
desc: Verify inheritance of multi-layer interfaces and classes to ensure correct access to constants and methods.
---*/

interface A {
    a: number;
    getA(): number;
}

abstract class B implements A {
    a: number = 3;
    abstract getB(): number;
    
    getA(): number {
        return this.a;
    }
}

class C extends B {
    getB(): number {
        return 4;
    }
}

function main(): void {
    const c = new C();
    assertEQ(c.getA(), 3);
    assertEQ(c.getB(), 4);
}