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
    A function parameter (except a parameter named 'this') is a valid
    left-hand-side expression and can be assigned to.
---*/

function setAndReturn(p: number): number {
    p = 5.0;
    return p;
}

function incrementAndReturn(p: int, delta: int): int {
    p = p + delta;
    return p;
}

function reassignString(s: string): string {
    s = "updated";
    return s;
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Parameter is valid LHS");

    suite.addTest("parameter can be assigned a new value", () => {
        arktest.assertEQ(setAndReturn(10.0), 5.0);
    });

    suite.addTest("parameter assignment does not affect caller's variable", () => {
        let x: number = 10.0;
        setAndReturn(x);
        arktest.assertEQ(x, 10.0);
    });

    suite.addTest("parameter can be incremented via assignment", () => {
        arktest.assertEQ(incrementAndReturn(10, 3), 13);
    });

    suite.addTest("string parameter can be reassigned", () => {
        arktest.assertEQ(reassignString("original"), "updated");
    });

    return suite.run();
}
