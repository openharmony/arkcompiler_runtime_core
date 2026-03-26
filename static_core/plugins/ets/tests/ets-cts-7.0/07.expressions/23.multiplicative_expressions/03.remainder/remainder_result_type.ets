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
  The result of remainder operation produces value of type int for byte, short
  or int operands, or long for long operands.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Remainder Result Type");

    suite.addTest("byte % byte produces int", () => {
        let a: byte = (10).toByte();
        let b: byte = (3).toByte();
        let r = a % b;
        let check: int = r;
        arktest.assertEQ(check, 1);
    });

    suite.addTest("short % short produces int", () => {
        let a: short = (10).toShort();
        let b: short = (3).toShort();
        let r = a % b;
        let check: int = r;
        arktest.assertEQ(check, 1);
    });

    suite.addTest("byte % short produces int", () => {
        let a: byte = (10).toByte();
        let b: short = (3).toShort();
        let r = a % b;
        let check: int = r;
        arktest.assertEQ(check, 1);
    });

    suite.addTest("int % int produces int", () => {
        let a: int = 10;
        let b: int = 3;
        let r = a % b;
        let check: int = r;
        arktest.assertEQ(check, 1);
    });

    suite.addTest("long % long produces long", () => {
        let a: long = 10;
        let b: long = 3;
        let r = a % b;
        let check: long = r;
        arktest.assertEQ(check, (1).toLong());
    });

    suite.addTest("int % long produces long", () => {
        let a: int = 10;
        let b: long = 3;
        let r = a % b;
        let check: long = r;
        arktest.assertEQ(check, (1).toLong());
    });

    suite.addTest("byte % long produces long", () => {
        let a: byte = (10).toByte();
        let b: long = 3;
        let r = a % b;
        let check: long = r;
        arktest.assertEQ(check, (1).toLong());
    });

    suite.addTest("float % float produces float", () => {
        let a: float = (10.0).toFloat();
        let b: float = (3.0).toFloat();
        let r = a % b;
        let check: float = r;
        arktest.assertTrue(check > (0.9).toFloat() && check < (1.1).toFloat());
    });

    suite.addTest("double % double produces double", () => {
        let a: double = 10.0;
        let b: double = 3.0;
        let r = a % b;
        let check: double = r;
        arktest.assertTrue(check > 0.9 && check < 1.1);
    });

    return suite.run();
}
