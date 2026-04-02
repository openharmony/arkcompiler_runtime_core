/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: Pass object literal without private field to Readonly type with multiple public fields
---*/

class A {
    public c = 25;
    public d = 26;
    private e = 27;
}

function takeReadonly(arg: Readonly<A>): int {
    return arg.c + arg.d;
}

function takePartialReadonly(arg: Partial<Readonly<A>>): int | undefined {
    return arg.c;
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Utility type private field - multiple public fields");

    suite.addTest("Pass object literal with all public fields to Readonly", () => {
        arktest.assertEQ(takeReadonly({ c: 28, d: 29 }), 57);
    });

    suite.addTest("Pass class instance to Readonly", () => {
        arktest.assertEQ(takeReadonly(new A()), 51);
    });

    suite.addTest("Pass partial object literal to Partial<Readonly>", () => {
        arktest.assertEQ(takePartialReadonly({ c: 30 }), 30);
    });

    return suite.run();
}
