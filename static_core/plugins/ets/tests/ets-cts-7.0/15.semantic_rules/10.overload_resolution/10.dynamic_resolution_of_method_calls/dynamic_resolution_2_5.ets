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
    A default method from a subinterface is chosen when the superinterface
    only declares an abstract method.
---*/

interface I1 {
    m(): string;
}

interface I2 extends I1 {
    m(): string {
        return "I2";
    }
}

class C implements I2 {
}

function main(): void {
    let i1: I1 = new C();

    arktest.assertEQ(i1.m(), "I2");
}
