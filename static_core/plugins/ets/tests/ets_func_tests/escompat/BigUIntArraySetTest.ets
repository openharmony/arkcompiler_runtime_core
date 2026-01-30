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

function main(): int {
    const suite = new arktest.ArkTestsuite('Test BigInt array set');

    suite.addTest("-1", () => {
        let testValue = -1n;

        let source = new BigInt64Array([testValue])
        let target = new BigUint64Array(1)
        target.set(source)

        arktest.assertEQ(18446744073709551615n, target[0])

    })

    suite.addTest("0", () => {
        let testValue = 0n;

        let source = new BigInt64Array([testValue])
        let target = new BigUint64Array(1)
        target.set(source)

        arktest.assertEQ(testValue, target[0])
    })

    suite.addTest("42", () => {
        let testValue = 42n;

        let source = new BigInt64Array([testValue])
        let target = new BigUint64Array(1)
        target.set(source)

        arktest.assertEQ(testValue, target[0])
    })

    suite.addTest("1", () => {
        let testValue = 1n;

        let source = new BigInt64Array([testValue])
        let target = new BigUint64Array(1)
        target.set(source)

        arktest.assertEQ(testValue, target[0])

    })

    suite.addTest("2^63-1", () => {
        let testValue = 9223372036854775807n;

        let source = new BigInt64Array([testValue])
        let target = new BigUint64Array(1)
        target.set(source)

        arktest.assertEQ(testValue, target[0])

    })

    suite.addTest("2^63", () => {
        let testValue = 9223372036854775808n;

        let source = new BigInt64Array([testValue])
        let target = new BigUint64Array(1)
        target.set(source)

        arktest.assertEQ(testValue, target[0])

    })

    suite.addTest("(2^63) + 1", () => {
        let testValue = 9223372036854775809n;

        let source = new BigInt64Array([testValue])
        let target = new BigUint64Array(1)
        target.set(source)

        arktest.assertEQ(testValue, target[0])

    })

    suite.addTest("(2^63) + (2^63)", () => {
        let testValue = 9223372036854775808n;
        testValue += testValue

        let source = new BigInt64Array([testValue])
        let target = new BigUint64Array(1)
        target.set(source)

        arktest.assertEQ(0n, target[0])

    })

    suite.addTest("-(2^63)", () => {
        let testValue = 9223372036854775808n;
        testValue = -testValue

        let source = new BigInt64Array([testValue])
        let target = new BigUint64Array(1)
        target.set(source)

        arktest.assertEQ(9223372036854775808n, target[0])

    })

    suite.addTest("-((2^63) + (2^63))", () => {
        let testValue = 9223372036854775808n;
        testValue += testValue
        testValue = -testValue

        let source = new BigInt64Array([testValue])
        let target = new BigUint64Array(1)
        target.set(source)

        arktest.assertEQ(0n, target[0])

    })
    return suite.run();
}
