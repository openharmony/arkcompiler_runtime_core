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
desc: Verify the correctness of the type system under multi-layer interface inheritance.
---*/

interface A {
    methodA(): string;
}

interface B extends A {
    methodB(): string;
}

interface C extends B {
    methodC(): string;
}

class D implements C {
    methodA(): string {
        return "A";
    }

    methodB(): string {
        return "B";
    }

    methodC(): string {
        return "C";
    }
}

function main(): void {
    const d: C = new D();
    assertEQ(d.methodA(), "A");
    assertEQ(d.methodB(), "B");
    assertEQ(d.methodC(), "C");
}