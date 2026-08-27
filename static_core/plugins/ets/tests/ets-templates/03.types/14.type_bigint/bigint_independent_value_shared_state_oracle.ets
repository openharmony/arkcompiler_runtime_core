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
desc: Bigint operations do not mutate independently created operand values.
test_unit: TEST-UNIT-03-0053
assertion: GAP-03-0084 / GRC-03-0110 / CI-03-14-009-01
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite('bigint independent values')
    suite.addTest('independent bigint values', () => {
        let first: bigint = BigInt(7)
        let second: bigint = BigInt(7)
        let firstResult: bigint = first + 1n

        arktest.assertTrue(first == 7n)
        arktest.assertTrue(second == 7n)
        arktest.assertTrue(firstResult == 8n)

        let secondResult: bigint = second + 2n

        arktest.assertTrue(first == 7n)
        arktest.assertTrue(second == 7n)
        arktest.assertTrue(secondResult == 9n)
    })
    return suite.run()
}
