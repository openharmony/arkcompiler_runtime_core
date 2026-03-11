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
  Operations on constant enumeration values: access, equality,
  relational comparisons, and arithmetic.
---*/

const enum Priority {
    Low = 1,
    Medium = 2,
    High = 3,
    Critical = 4
}

const enum Status {
    Active,
    Inactive,
    Pending
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Constant Enumeration Operations")

    suite.addTest("Access const enum member value", () => {
        let p: Priority = Priority.High
        arktest.assertEQ(p.valueOf(), 3)
    })

    suite.addTest("Equality operator ==", () => {
        let a = Status.Active
        let b = Status.Active
        arktest.assertTrue(a == b)
    })

    suite.addTest("Inequality operator !=", () => {
        let a = Status.Active
        let b = Status.Inactive
        arktest.assertTrue(a != b)
    })

    suite.addTest("Strict equality ===", () => {
        let a = Priority.Medium
        let b = Priority.Medium
        arktest.assertTrue(a === b)
    })

    suite.addTest("Strict inequality !==", () => {
        let a = Priority.Low
        let b = Priority.High
        arktest.assertTrue(a !== b)
    })

    suite.addTest("Less than <", () => {
        arktest.assertTrue(Priority.Low < Priority.Medium)
        arktest.assertFalse(Priority.High < Priority.Low)
    })

    suite.addTest("Less than or equal <=", () => {
        arktest.assertTrue(Priority.Low <= Priority.Low)
        arktest.assertTrue(Priority.Low <= Priority.Medium)
        arktest.assertFalse(Priority.High <= Priority.Low)
    })

    suite.addTest("Greater than >", () => {
        arktest.assertTrue(Priority.Critical > Priority.Low)
        arktest.assertFalse(Priority.Low > Priority.High)
    })

    suite.addTest("Greater than or equal >=", () => {
        arktest.assertTrue(Priority.High >= Priority.High)
        arktest.assertTrue(Priority.Critical >= Priority.Medium)
        arktest.assertFalse(Priority.Low >= Priority.Medium)
    })

    suite.addTest("Arithmetic on const enum values", () => {
        let sum = Priority.Low + Priority.Medium
        arktest.assertEQ(sum, 3)
        let diff = Priority.High - Priority.Low
        arktest.assertEQ(diff, 2)
    })

    suite.addTest("Const enum in conditional expression", () => {
        let p = Priority.Critical
        let label = p == Priority.Critical ? "urgent" : "normal"
        arktest.assertEQ(label, "urgent")
    })

    return suite.run()
}
