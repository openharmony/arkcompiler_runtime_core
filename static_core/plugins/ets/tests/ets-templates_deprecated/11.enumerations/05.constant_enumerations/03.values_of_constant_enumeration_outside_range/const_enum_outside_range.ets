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
  A variable of constant enumeration type can be assigned a value
  outside the range of defined members via explicit cast.
  A switch default case handles such out-of-range values.
---*/

const enum Level {
    Low = 1,
    Medium = 2,
    High = 3
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Values of Constant Enumeration Outside Range")

    suite.addTest("Assign non-member value via explicit cast", () => {
        let x: Level = 99 as Level
        arktest.assertEQ(x.valueOf(), 99)
    })

    suite.addTest("Switch with default for outside-range value", () => {
        let x: Level = 99 as Level
        let result = ""
        switch (x) {
            case Level.Low: result = "low"; break
            case Level.Medium: result = "medium"; break
            case Level.High: result = "high"; break
            default: result = "unknown"; break
        }
        arktest.assertEQ(result, "unknown")
    })

    suite.addTest("Outside-range value is not equal to any member", () => {
        let x: Level = 50 as Level
        arktest.assertTrue(x != Level.Low)
        arktest.assertTrue(x != Level.Medium)
        arktest.assertTrue(x != Level.High)
    })

    suite.addTest("Negative non-member value via explicit cast", () => {
        let x: Level = -10 as Level
        arktest.assertEQ(x.valueOf(), -10)
    })

    return suite.run()
}
