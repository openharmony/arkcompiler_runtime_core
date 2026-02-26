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
  Const enumeration relational operators: both operands of the same const
  enumeration type can be compared using relational operators <, <=, >, >=.
---*/

const enum NumericEnum {
    First = 0,
    Second = 1,
    Third = 2
}

const enum StringEnum {
    Alpha = "A",
    Beta = "B",
    Gamma = "C"
}

const enum AllSame {
    X = 11,
    Y = 11,
    Z = 11
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Const Enum Relational");

    suite.addTest("numeric const enum: less than", () => {
        arktest.assertTrue(NumericEnum.First < NumericEnum.Second);
        arktest.assertTrue(NumericEnum.Second < NumericEnum.Third);
    });

    suite.addTest("numeric const enum: less than or equal", () => {
        arktest.assertTrue(NumericEnum.First <= NumericEnum.Second);
        arktest.assertTrue(NumericEnum.First <= NumericEnum.First);
    });

    suite.addTest("numeric const enum: greater than", () => {
        arktest.assertTrue(NumericEnum.Third > NumericEnum.Second);
        arktest.assertTrue(NumericEnum.Second > NumericEnum.First);
    });

    suite.addTest("numeric const enum: greater than or equal", () => {
        arktest.assertTrue(NumericEnum.Third >= NumericEnum.Second);
        arktest.assertTrue(NumericEnum.Third >= NumericEnum.Third);
    });

    suite.addTest("numeric const enum: not less than when equal", () => {
        arktest.assertFalse(AllSame.X < AllSame.Y);
        arktest.assertFalse(AllSame.Y > AllSame.Z);
    });

    suite.addTest("numeric const enum: <= and >= when equal", () => {
        arktest.assertTrue(AllSame.X <= AllSame.Y);
        arktest.assertTrue(AllSame.Y >= AllSame.Z);
    });

    suite.addTest("string const enum: less than", () => {
        arktest.assertTrue(StringEnum.Alpha < StringEnum.Beta);
        arktest.assertTrue(StringEnum.Beta < StringEnum.Gamma);
    });

    suite.addTest("string const enum: less than or equal", () => {
        arktest.assertTrue(StringEnum.Alpha <= StringEnum.Beta);
        arktest.assertTrue(StringEnum.Alpha <= StringEnum.Alpha);
    });

    suite.addTest("string const enum: greater than", () => {
        arktest.assertTrue(StringEnum.Gamma > StringEnum.Beta);
        arktest.assertTrue(StringEnum.Beta > StringEnum.Alpha);
    });

    suite.addTest("string const enum: greater than or equal", () => {
        arktest.assertTrue(StringEnum.Gamma >= StringEnum.Beta);
        arktest.assertTrue(StringEnum.Gamma >= StringEnum.Gamma);
    });

    suite.addTest("numeric const enum: relational through variables", () => {
        let a: NumericEnum = NumericEnum.First;
        let b: NumericEnum = NumericEnum.Third;
        arktest.assertTrue(a < b);
        arktest.assertTrue(b > a);
        arktest.assertTrue(a <= b);
        arktest.assertTrue(b >= a);
    });

    suite.addTest("string const enum: relational through variables", () => {
        let a: StringEnum = StringEnum.Alpha;
        let b: StringEnum = StringEnum.Gamma;
        arktest.assertTrue(a < b);
        arktest.assertTrue(b > a);
        arktest.assertTrue(a <= b);
        arktest.assertTrue(b >= a);
    });

    return suite.run();
}
