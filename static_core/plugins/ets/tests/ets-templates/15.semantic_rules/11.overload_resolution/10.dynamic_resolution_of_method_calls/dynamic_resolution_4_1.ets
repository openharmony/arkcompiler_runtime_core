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
    A class method resolves a diamond conflict between two default methods
    inherited from subinterfaces.
---*/

interface I1 {
    m(): string {
        return "I1";
    }
}

interface I2 extends I1 {
    m(): string {
        return "I2";
    }
}

interface I3 extends I1 {
    m(): string {
        return "I3";
    }
}

class C implements I2, I3 {
    m(): string {
        return "C";
    }
}

function main(): void {
    let i1: I1 = new C();
    let i2: I2 = new C();
    let i3: I3 = new C();

    arktest.assertEQ(i1.m(), "C");
    arktest.assertEQ(i2.m(), "C");
    arktest.assertEQ(i3.m(), "C");
}
