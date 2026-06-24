/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: >-
    Method inherited from superclass is chosen over interface default method
    when the subclass implements the interface without override.
---*/

interface I {
    m(): string {
        return "I";
    }
}

class A {
    m(): string {
        return "A";
    }
}

class B extends A implements I {
}

function main(): void {
    let i: I = new B();
    arktest.assertEQ(i.m(), "A");
}
