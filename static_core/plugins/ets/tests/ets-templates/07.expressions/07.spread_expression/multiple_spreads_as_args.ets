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
    If a spread expression is used to pass arguments, then the sequence of
    spread expressions passed as sequential arguments yields a single sequence
    of values.
---*/

function collect(...args: number[]): number[] {
    return args
}

function g1(): number[] { return [1, 2] }
function g2(): number[] { return [3, 4, 5] }

function main(): int {
    let suite = new arktest.ArkTestsuite("7.7 Multiple spread expressions as sequential arguments")

    suite.addTest("Two spread arrays as direct args to rest parameter", () => {
        let arr: number[] = [1, 2, 3]
        let result = collect(...arr, ...arr)
        arktest.assertEQ(result.length, 6)
        arktest.assertEQ(result[0], 1.0)
        arktest.assertEQ(result[3], 1.0)
    })

    suite.addTest("Spread function return values as direct args", () => {
        let result = collect(...g1(), ...g2())
        arktest.assertEQ(result.length, 5)
        arktest.assertEQ(result[0], 1.0)
        arktest.assertEQ(result[4], 5.0)
    })

    suite.addTest("Three spread arrays including empty", () => {
        let a: number[] = [1.0, 2.0]
        let b: number[] = []
        let c: number[] = [3.0]
        let result = collect(...a, ...b, ...c)
        arktest.assertEQ(result.length, 3)
        arktest.assertEQ(result[2], 3.0)
    })

    return suite.run()
}
