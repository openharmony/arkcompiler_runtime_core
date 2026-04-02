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
  Const enumeration equality: both operands of the same const enumeration type
  can be compared using equality operators ==, ===, !=, !==.
---*/

const enum NumericEnum {
    First = 0,
    Second = 1,
    Third = 0
}

const enum StringEnum {
    Alpha = "A",
    Beta = "B",
    Gamma = "A"
}

const enum AllSame {
    X = 11,
    Y = 11,
    Z = 11
}

const enum AllSameStr {
    X = "ABC",
    Y = "ABC",
    Z = "ABC"
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Const Enum Equality");

    suite.addTest("numeric const enum: same values are equal with ==", () => {
        arktest.assertTrue(NumericEnum.First == NumericEnum.Third);
    });

    suite.addTest("numeric const enum: different values are not equal with ==", () => {
        arktest.assertTrue(NumericEnum.First != NumericEnum.Second);
    });

    suite.addTest("numeric const enum: same values are equal with ===", () => {
        arktest.assertTrue(NumericEnum.First === NumericEnum.Third);
    });

    suite.addTest("numeric const enum: different values are not equal with !==", () => {
        arktest.assertTrue(NumericEnum.First !== NumericEnum.Second);
    });

    suite.addTest("string const enum: same values are equal with ==", () => {
        arktest.assertTrue(StringEnum.Alpha == StringEnum.Gamma);
    });

    suite.addTest("string const enum: different values are not equal with ==", () => {
        arktest.assertTrue(StringEnum.Alpha != StringEnum.Beta);
    });

    suite.addTest("string const enum: same values are equal with ===", () => {
        arktest.assertTrue(StringEnum.Alpha === StringEnum.Gamma);
    });

    suite.addTest("string const enum: different values are not equal with !==", () => {
        arktest.assertTrue(StringEnum.Alpha !== StringEnum.Beta);
    });

    suite.addTest("numeric const enum: all same values are equal", () => {
        arktest.assertTrue(AllSame.X == AllSame.Y && AllSame.Y == AllSame.Z && AllSame.X == AllSame.Z);
    });

    suite.addTest("string const enum: all same values are equal", () => {
        arktest.assertTrue(AllSameStr.X == AllSameStr.Y && AllSameStr.Y == AllSameStr.Z && AllSameStr.X == AllSameStr.Z);
    });

    suite.addTest("numeric const enum: equality through variables", () => {
        let a: NumericEnum = NumericEnum.First;
        let b: NumericEnum = NumericEnum.Third;
        arktest.assertTrue(a == b);
    });

    suite.addTest("string const enum: equality through variables", () => {
        let a: StringEnum = StringEnum.Alpha;
        let b: StringEnum = StringEnum.Gamma;
        arktest.assertTrue(a == b);
    });

    suite.addTest("numeric const enum: inequality through variables", () => {
        let a: NumericEnum = NumericEnum.First;
        let b: NumericEnum = NumericEnum.Second;
        arktest.assertTrue(a != b);
    });

    suite.addTest("string const enum: inequality through variables", () => {
        let a: StringEnum = StringEnum.Alpha;
        let b: StringEnum = StringEnum.Beta;
        arktest.assertTrue(a != b);
    });

    suite.addTest("numeric const enum: equality through array access", () => {
        let arr: NumericEnum[] = [NumericEnum.First, NumericEnum.Second, NumericEnum.Third];
        arktest.assertTrue(arr[0] == arr[2]);
        arktest.assertTrue(arr[0] != arr[1]);
    });

    suite.addTest("string const enum: equality through array access", () => {
        let arr: StringEnum[] = [StringEnum.Alpha, StringEnum.Beta, StringEnum.Gamma];
        arktest.assertTrue(arr[0] == arr[2]);
        arktest.assertTrue(arr[0] != arr[1]);
    });

    return suite.run();
}
