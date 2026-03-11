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
  Constant enumeration members initialization: implicit int values,
  string values, bitwise/arithmetic computed values.
---*/

const enum Direction {
    Up,
    Down,
    Left,
    Right
}

const enum Flags {
    None = 0,
    Read = 1,
    Write = 2,
    Execute = 4,
    ReadWrite = 1 | 2,
    All = 1 | 2 | 4
}

const enum Computed {
    A = 10,
    B = 20,
    C = A + B,
    D = A * 3
}

const enum StringColors {
    Red = "RED",
    Green = "GREEN",
    Blue = "BLUE"
}

const enum Mixed {
    First = 100,
    Second,
    Third
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Constant Enumeration Members Initialization")

    suite.addTest("Implicit int values start from 0", () => {
        arktest.assertEQ(Direction.Up.valueOf(), 0)
        arktest.assertEQ(Direction.Down.valueOf(), 1)
        arktest.assertEQ(Direction.Left.valueOf(), 2)
        arktest.assertEQ(Direction.Right.valueOf(), 3)
    })

    suite.addTest("Const enum member is instanceof its type", () => {
        arktest.assertTrue(Direction.Up instanceof Direction)
        arktest.assertTrue(Direction.Right instanceof Direction)
    })

    suite.addTest("Bitwise OR computed initializers", () => {
        arktest.assertEQ(Flags.None.valueOf(), 0)
        arktest.assertEQ(Flags.Read.valueOf(), 1)
        arktest.assertEQ(Flags.Write.valueOf(), 2)
        arktest.assertEQ(Flags.Execute.valueOf(), 4)
        arktest.assertEQ(Flags.ReadWrite.valueOf(), 3)
        arktest.assertEQ(Flags.All.valueOf(), 7)
    })

    suite.addTest("Arithmetic computed initializers using other members", () => {
        arktest.assertEQ(Computed.A.valueOf(), 10)
        arktest.assertEQ(Computed.B.valueOf(), 20)
        arktest.assertEQ(Computed.C.valueOf(), 30)
        arktest.assertEQ(Computed.D.valueOf(), 30)
    })

    suite.addTest("String const enum values", () => {
        arktest.assertEQ(StringColors.Red.valueOf(), "RED")
        arktest.assertEQ(StringColors.Green.valueOf(), "GREEN")
        arktest.assertEQ(StringColors.Blue.valueOf(), "BLUE")
    })

    suite.addTest("Implicit values continue from explicit start", () => {
        arktest.assertEQ(Mixed.First.valueOf(), 100)
        arktest.assertEQ(Mixed.Second.valueOf(), 101)
        arktest.assertEQ(Mixed.Third.valueOf(), 102)
    })

    return suite.run()
}
