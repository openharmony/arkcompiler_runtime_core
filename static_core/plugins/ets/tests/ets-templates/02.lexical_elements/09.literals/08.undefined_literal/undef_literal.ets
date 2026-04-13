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
desc: Undefined literal
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Undefined literal");

    suite.addTest("Implicit type", () => {
        let x = undefined; // x type is expected to be inferred to Object|undefined type
        arktest.assertTrue( x === undefined )
    });

    suite.addTest("Explicit type", () => {
        let x: undefined = undefined;
        arktest.assertTrue( x === undefined )
    });

    suite.addTest("Union type", () => {
        let y: Object|undefined = undefined;
        arktest.assertTrue( y === undefined )
    });

    suite.addTest("Standalone literal", () => {
        undefined //standalone literal
        arktest.assertTrue(undefined instanceof undefined)
    });

    return suite.run();
}
