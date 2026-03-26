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
desc: widening numeric to a union type 
---*/

function main(): int {
    let myTestsuite = new arktest.ArkTestsuite("widening short to a union type");

    myTestsuite.addTest("widening to int", () => {
        let n: short = 12
        let u: int | boolean

        u = n 

        arktest.assertTrue(u instanceof int, "incorrect type after widening to a union type")
        arktest.assertEQ(u, n, "numeric value changed after widening to a union type")
    });
    
    myTestsuite.addTest("widening to long", () => {
        let n: short = -10
        let u: long | null

        u = n 

        arktest.assertTrue(u instanceof long, "incorrect type after widening to a union type")
        arktest.assertEQ(u, n, "numeric value changed after widening to a union type")
    });

    myTestsuite.addTest("widening to float", () => {
        let n: short = 99
        let u: string | float | never

        u = n 

        arktest.assertTrue(u instanceof float, "incorrect type after widening to a union type")
        arktest.assertEQ(u, n, "numeric value changed after widening to a union type")
    });

    myTestsuite.addTest("widening to double", () => {
        let n: short = 0
        let u: Array<short> | double

        u = n 

        arktest.assertTrue(u instanceof double, "incorrect type after widening to a union type")
        arktest.assertEQ(u, n, "numeric value changed after widening to a union type")
    });

    return myTestsuite.run();
}
