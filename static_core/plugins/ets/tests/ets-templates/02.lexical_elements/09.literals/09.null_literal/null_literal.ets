/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: Null literal.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Null literal");

    const eq = (a: Any, b: Any) => a == b
    const strictEq = (a: Any, b: Any) => a === b
    const instanceOf = (a: Any) => a instanceof null

    suite.addTest("Implicit type", () => {
        let x = null; // x type is expected to be inferred to Object|null type
        arktest.assertTrue(x == null)
        arktest.assertTrue(x === null)
        arktest.assertTrue(x instanceof null)
        arktest.assertTrue(eq(x, null))
        arktest.assertTrue(eq(null, x))
        arktest.assertTrue(strictEq(x, null))
        arktest.assertTrue(strictEq(null, x))
        arktest.assertTrue(instanceOf(x))
    });

    suite.addTest("Explicit type", () => {
        let x: null = null;
        arktest.assertTrue(x == null)
        arktest.assertTrue(x === null)
        arktest.assertTrue(x instanceof null)
        arktest.assertTrue(eq(x, null))
        arktest.assertTrue(eq(null, x))
        arktest.assertTrue(strictEq(x, null))
        arktest.assertTrue(strictEq(null, x))
        arktest.assertTrue(instanceOf(x))
    });

    suite.addTest("Union type", () => {
        let x: Object | null = null;
        arktest.assertTrue(x == null)
        arktest.assertTrue(x === null)
        arktest.assertTrue(x instanceof null)
        arktest.assertTrue(eq(x, null))
        arktest.assertTrue(eq(null, x))
        arktest.assertTrue(strictEq(x, null))
        arktest.assertTrue(strictEq(null, x))
        arktest.assertTrue(instanceOf(x))
    });

    suite.addTest("Standalone literal", () => {
        null //standalone literal
        arktest.assertTrue(null == null)
        arktest.assertTrue(null === null)
        arktest.assertTrue(null instanceof null)
        arktest.assertTrue(eq(null, null))
        arktest.assertTrue(strictEq(null, null))
        arktest.assertTrue(instanceOf(null))
    });

    return suite.run();
}
