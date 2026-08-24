/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: |-
    - If the value of condition is evaluated as true at compile time, then the expression has the typeof whenTrue.
    - If the value of condition is evaluated as false at compile time, then the expression has the typeof whenFalse.
    - If the value of condition is unknown at compile time, then the expression type is a union of types whenTrue
    and whenFalse further normalized in accordance with the process discussed in Union TypesNormalization.
---*/

let trueVar = true

let v1: boolean|null|undefined = null
let v2: string|double|boolean = 42.0

function main(): int {
      const trueConst = true
      let suite = new arktest.ArkTestsuite('Two operands of different types')

      suite.addTest('Explicit int type of expression', () => {
          let x: int = trueConst ? 4 : 'hello'
          arktest.assertEQ(x, 4)
          x = !trueConst ? 'hello' : 5
          arktest.assertEQ(x, 5)
      })

      suite.addTest('Inferred int type of expression', () => {
          let x = trueConst ? 4 : 'hello'
          arktest.assertEQ(x, 4)
          x = !trueConst ? 'hello' : 5
          arktest.assertEQ(x, 5)
      })

      suite.addTest('Explicit int|string type of expression', () => {
          let x: int|string = trueVar ? 4 : 'hello'
          arktest.assertEQ(x, 4)
          x = !trueVar ? 5 : 'hello'
          arktest.assertEQ(x, 'hello')
      })

      suite.addTest('Inferred int|string type of expression', () => {
          let x = trueVar ? 4 : 'hello'
          arktest.assertEQ(x, 4)
          x = !trueVar ? 5 : 'hello'
          arktest.assertEQ(x, 'hello')
      })

      suite.addTest('Explicit boolean|null|undefined|string|double type of expression', () => {
          let x: boolean|null|undefined|string|double = trueVar ? v1 : v2
          arktest.assertEQ(x, null)
          x = !trueVar ? v1 : v2
          arktest.assertEQ(x, 42.0)
          // check union type by assignability
          arktest.assertEQ(x = true, true)
          arktest.assertEQ(x = undefined, undefined)
          arktest.assertEQ(x = 'hello', 'hello')
      })

      suite.addTest('Inferred boolean|null|undefined|string|double type of expression', () => {
          let x = trueVar ? v1 : v2
          arktest.assertEQ(x, null)
          x = !trueVar ? v1 : v2
          arktest.assertEQ(x, 42.0)
          // check union type by assignability
          arktest.assertEQ(x = true, true)
          arktest.assertEQ(x = undefined, undefined)
          arktest.assertEQ(x = 'hello', 'hello')
      })

      return suite.run()
}
