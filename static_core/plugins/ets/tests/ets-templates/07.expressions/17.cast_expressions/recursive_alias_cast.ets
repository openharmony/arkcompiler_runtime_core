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
desc: >-
    Cast expressions correctly resolve recursive generic type aliases,
    including aliases with rotated type parameters and mutual recursion.
---*/

class Cell<T> {
  value: T | null

  constructor(value: T | null) {
    this.value = value
  }
}

type Rotating<T, C, U> = T | T[] | Rotating<C, U, T>[]
type M<T> = Cell<F<T>>
type F<T> = Cell<M<T>>

function main(): int {
  let suite = new arktest.ArkTestsuite("Recursive Alias Cast Expressions")

  suite.addTest("Rotating recursive alias cast keeps substitutions", () => {
    let seed: Rotating<string, Int, Boolean> = "test"
    let value: Rotating<string, Int, Boolean> =
      [1 as Int, 2 as Int,
       [true as Boolean, false as Boolean,
        [seed] as Rotating<string, Int, Boolean>[]] as Rotating<Boolean, string, Int>[]] as Rotating<Int, Boolean, string>[]

    let rotated = value as Rotating<Int, Boolean, string>[]
    arktest.assertEQ(rotated[0] as Int, 1 as Int)
    arktest.assertEQ(rotated[1] as Int, 2 as Int)

    let nested = rotated[2] as Rotating<Boolean, string, Int>[]
    arktest.assertEQ(nested[0], true as Boolean)
    arktest.assertEQ(nested[1], false as Boolean)
    arktest.assertEQ((nested[2] as Rotating<string, Int, Boolean>[])[0], "test")
  })

  suite.addTest("Mutual recursive alias cast resolves nested aliases", () => {
    let innerMost: M<number> = new Cell<F<number>>(null)
    let middle: F<number> = new Cell<M<number>>(innerMost)
    let value: M<number> = new Cell<F<number>>(middle)

    let first = value as Cell<F<number>>
    let second = first.value as F<number>
    let third = (second as Cell<M<number>>).value as M<number>
    let fourth = third as Cell<F<number>>

    arktest.assertTrue(first.value != null)
    arktest.assertTrue((second as Cell<M<number>>).value != null)
    arktest.assertEQ(fourth.value, null)
  })

  return suite.run()
}
