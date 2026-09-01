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
    Nullish-coalescing assignment evaluates its left-hand side once and evaluates and assigns
    its right-hand side only when the saved left-hand-side value is null or undefined.
test_unit: TEST-UNIT-07-0001
assertion: GAP-07-0001 / GRC-07-0001 / CI-07-32-011-01
---*/

class Holder {
    public value: string | null | undefined

    constructor(value: string | null | undefined) {
        this.value = value
    }
}

let lhsEvaluations: int = 0
let rhsEvaluations: int = 0

function select(holder: Holder): Holder {
    lhsEvaluations++
    return holder
}

function replacement(): string {
    rhsEvaluations++
    return 'replacement'
}

function throwingReplacement(): string {
    rhsEvaluations++
    throw new Error('right-hand side failed')
}

function resetCounters(): void {
    lhsEvaluations = 0
    rhsEvaluations = 0
}

function main(): int {
    let suite = new arktest.ArkTestsuite('nullish-coalescing assignment')

    suite.addTest('non-null left value skips the right-hand side', () => {
        resetCounters()
        let holder = new Holder('kept')

        let temp = select(holder)
        if (temp.value === null || temp.value === undefined) {
            temp.value = replacement()
        }

        arktest.assertEQ(holder.value, 'kept')
        arktest.assertEQ(lhsEvaluations, 1)
        arktest.assertEQ(rhsEvaluations, 0)
    })

    suite.addTest('null left value evaluates and assigns the right-hand side', () => {
        resetCounters()
        let holder = new Holder(null)

        let temp = select(holder)
        if (temp.value === null || temp.value === undefined) {
            temp.value = replacement()
        }

        arktest.assertEQ(holder.value, 'replacement')
        arktest.assertEQ(lhsEvaluations, 1)
        arktest.assertEQ(rhsEvaluations, 1)
    })

    suite.addTest('undefined left value evaluates and assigns the right-hand side', () => {
        resetCounters()
        let holder = new Holder(undefined)

        let temp = select(holder)
        if (temp.value === null || temp.value === undefined) {
            temp.value = replacement()
        }

        arktest.assertEQ(holder.value, 'replacement')
        arktest.assertEQ(lhsEvaluations, 1)
        arktest.assertEQ(rhsEvaluations, 1)
    })

    suite.addTest('abrupt right-hand side does not assign', () => {
        resetCounters()
        let holder = new Holder(null)
        let expectedException = (error: Error): boolean => error instanceof Error

        arktest.expectThrow(() => {
            let temp = select(holder)
            if (temp.value === null || temp.value === undefined) {
                temp.value = throwingReplacement()
            }
        }, expectedException)

        arktest.assertTrue(holder.value === null)
        arktest.assertEQ(lhsEvaluations, 1)
        arktest.assertEQ(rhsEvaluations, 1)
    })

    return suite.run()
}
