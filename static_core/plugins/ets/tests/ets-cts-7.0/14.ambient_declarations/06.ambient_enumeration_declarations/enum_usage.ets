/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Ambient enumeration declarations can be used after import
files: ['./enum_declarations.ets']
---*/

import { Color, Direction, SingleMember, Severity } from './enum_declarations.d'

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

  return suite.run()
}
