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
desc: Pass object literal without private field to Readonly type with static public field
---*/

class A {
    public static a = 64;
    public b = 65;
    private c = 66;
}

function takeReadonly(arg: Readonly<A>): int {
    return arg.b;
}

function takePartialReadonly(arg: Partial<Readonly<A>>): int | undefined {
    return arg.b;
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Utility type private field - static field");

    suite.addTest("Pass object literal without private field to Readonly", () => {
        arktest.assertEQ(takeReadonly({ b: 67 }), 67);
    });

    suite.addTest("Pass class instance to Readonly", () => {
        arktest.assertEQ(takeReadonly(new A()), 65);
    });

    suite.addTest("Pass object literal without private field to Partial<Readonly>", () => {
        arktest.assertEQ(takePartialReadonly({ b: 68 }), 68);
    });

    return suite.run();
}
