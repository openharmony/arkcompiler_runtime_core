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
desc: Ambient enumeration declarations can be used after import
files: ['./enum_declarations.ets']
---*/

import { Color, Direction, SingleMember, Severity,
    IntEnum, StrEnum, MixedInit, ByteEnum } from './enum_declarations.d'

function main(): int {
  let suite = new arktest.ArkTestsuite("Ambient Enum Declarations")

  suite.addTest("Basic ambient enum members have sequential values", () => {
    arktest.assertEQ(Color.Red, 0)
    arktest.assertEQ(Color.Green, 1)
    arktest.assertEQ(Color.Blue, 2)
  })

  suite.addTest("Ambient enum with trailing comma in declaration", () => {
    arktest.assertEQ(Direction.Up, 0)
    arktest.assertEQ(Direction.Down, 1)
    arktest.assertEQ(Direction.Left, 2)
    arktest.assertEQ(Direction.Right, 3)
  })

  suite.addTest("Ambient enum with single member", () => {
    arktest.assertEQ(SingleMember.Only, 0)
  })

  suite.addTest("Ambient enum with explicit base type", () => {
    arktest.assertEQ(Severity.Low, (0).toByte())
    arktest.assertEQ(Severity.Medium, (1).toByte())
    arktest.assertEQ(Severity.High, (2).toByte())
  })

  suite.addTest("Ambient enum with int initializers", () => {
    arktest.assertEQ(IntEnum.A.valueOf(), 10)
    arktest.assertEQ(IntEnum.B.valueOf(), 20)
    arktest.assertEQ(IntEnum.C.valueOf(), 30)
  })

  suite.addTest("Ambient enum with string initializers", () => {
    arktest.assertEQ(StrEnum.X.valueOf(), "x")
    arktest.assertEQ(StrEnum.Y.valueOf(), "y")
  })

  suite.addTest("Ambient enum with partial initializers", () => {
    arktest.assertEQ(MixedInit.A.valueOf(), 0);
    arktest.assertEQ(MixedInit.B.valueOf(), 5);
    arktest.assertEQ(MixedInit.C.valueOf(), 6)
  })

  suite.addTest("Ambient enum with explicit base type and initializers", () => {
    arktest.assertEQ(ByteEnum.Lo.valueOf(), 1 as byte)
    arktest.assertEQ(ByteEnum.Hi.valueOf(), 127 as byte)
  })

  return suite.run()
}
