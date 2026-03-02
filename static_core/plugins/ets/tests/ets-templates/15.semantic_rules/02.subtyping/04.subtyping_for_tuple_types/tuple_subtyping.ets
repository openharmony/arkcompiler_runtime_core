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
    Any tuple type is a subtype of type Tuple.
    Tuple type T (P1, ..., Pn) is a subtype of type S (P1, ..., Pm),
    where n >= m, i.e., a tuple type is a subtype of a tuple type
    with fewer identical constituent types.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Subtyping for Tuple Types")

    suite.addTest("any tuple is subtype of empty tuple (Tuple)", () => {
        let t: [string, number] = ["hello", 1.0]
        let e: [] = t
        arktest.assertTrue(e instanceof Object)
    })

    suite.addTest("longer tuple is subtype of shorter with identical prefix types", () => {
        let d: [string, number, boolean] = ["a", 1.0, true]
        let t: [string, number] = d
        arktest.assertEQ(t[0], "a")
        arktest.assertEQ(t[1], 1.0)
    })

    suite.addTest("two-element tuple is subtype of single-element with identical first type", () => {
        let t2: [string, number] = ["hello", 1.0]
        let t1: [string] = t2
        arktest.assertEQ(t1[0], "hello")
    })

    return suite.run()
}
