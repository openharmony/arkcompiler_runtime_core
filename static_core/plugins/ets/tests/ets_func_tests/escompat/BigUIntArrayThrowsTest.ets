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
desc: BigInt array exceptions check
---*/

 function main(): int {
    const suite = new arktest.ArkTestsuite("Test exceptions set with bad target offset")

    suite.addTest("BigInt64Array 0", () => {
        let testee = new BigInt64Array(4)
        arktest.expectThrow(() => {
            testee.set(testee, -1)
          });
    })
    suite.addTest("BigUint64Array 0", () => {
        let testee = new BigUint64Array(4)
        arktest.expectThrow(() => {
            testee.set(testee, -1)
          });
    })

    suite.addTest("BigInt64Array 1", () => {
        let testee = new BigInt64Array(4)
        arktest.expectThrow(() => {
            testee.set([1n], -1)
          });
    })
    suite.addTest("BigUint64Array 1", () => {
        let testee = new BigUint64Array(4)
        arktest.expectThrow(() => {
            testee.set([1n], -1)
          });
    })

    suite.addTest("BigUint64Array 2", () => {
        let source = new BigUint64Array(2)
        let testee = new BigUint64Array(2)
        arktest.expectThrow(() => {
            testee.set(source, 1)
          });
    })

    suite.addTest("BigUint64Array 3", () => {
        let source = new BigUint64Array(2)
        let testee = new BigUint64Array(1)
        arktest.expectThrow(() => {
            testee.set(source, 0)
          });
    })

    suite.addTest("BigUint64Array 4: (2 + 0 > 1)", () => {
        let source = new BigUint64Array(0)
        let testee = new BigUint64Array(1)
        arktest.expectThrow(() => {
            testee.set(source, 2)
          });
    })

    suite.addTest("BigInt64Array 2", () => {
        let source = new BigInt64Array(2)
        let testee = new BigInt64Array(2)
        arktest.expectThrow(() => {
            testee.set(source, 1)
          });
    })

    suite.addTest("BigInt64Array 3", () => {
        let source = new BigInt64Array(2)
        let testee = new BigInt64Array(1)
        arktest.expectThrow(() => {
            testee.set(source, 0)
          });
    })

    suite.addTest("BigInt64Array 4: (2 + 0 > 1)", () => {
        let source = new BigInt64Array(0)
        let testee = new BigInt64Array(1)
        arktest.expectThrow(() => {
            testee.set(source, 2)
          });
    })

    return suite.run();
 }
