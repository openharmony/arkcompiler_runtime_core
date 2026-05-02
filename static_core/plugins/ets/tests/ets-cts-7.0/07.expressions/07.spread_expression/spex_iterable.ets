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
desc: A spread expression can be used only within the array literal or argument passing.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Spread of built-in iterables")

    suite.addTest("iterable of strings in spread expression", () => {
        let a: Iterable<string> = 'abc'
        let r: Array<string> = [...a, ...a]
        arktest.assertEQ(r.length, 6)
        arktest.assertEQ(r[0], 'a')
        arktest.assertEQ(r[1], 'b')
        arktest.assertEQ(r[2], 'c')
        arktest.assertEQ(r[3], 'a')
        arktest.assertEQ(r[4], 'b')
        arktest.assertEQ(r[5], 'c')
    })

    suite.addTest("iterable of tuples in spread expression", () => {
        let a: Iterable<[int, string]> = {
            1: 'a',
            2: 'b',
        } as Record<int, string>
        let r: Array<[int, string]> = [...a, ...a]
        arktest.assertEQ(r.length, 4)
        arktest.assertEQ(r[0][0], 1)
        arktest.assertEQ(r[0][1], 'a')
        arktest.assertEQ(r[1][0], 2)
        arktest.assertEQ(r[1][1], 'b')
        arktest.assertEQ(r[2][0], 1)
        arktest.assertEQ(r[2][1], 'a')
        arktest.assertEQ(r[3][0], 2)
        arktest.assertEQ(r[3][1], 'b')
    })

    return suite.run()
}
