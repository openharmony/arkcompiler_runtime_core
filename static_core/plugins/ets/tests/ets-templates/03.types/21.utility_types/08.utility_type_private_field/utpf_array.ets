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
desc: Pass object literal without private field to Readonly type with array type public field
---*/

class A {
    public a: number[] = [1, 2];
    private b: string = "private array";
}

function takeReadonly(arg: Readonly<A>): number {
    return arg.a[0];
}

function takePartialReadonly(arg: Partial<Readonly<A>>): number | undefined {
    return arg.a?.[0];
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Utility type private field - array fields");

    suite.addTest("Pass object literal without private field to Readonly", () => {
        arktest.assertEQ(takeReadonly({ a: [3, 4] }), 3);
    });

    suite.addTest("Pass class instance to Readonly", () => {
        arktest.assertEQ(takeReadonly(new A()), 1);
    });

    suite.addTest("Pass object literal without private field to Partial<Readonly>", () => {
        arktest.assertEQ(takePartialReadonly({ a: [5, 6] }), 5);
    });

    return suite.run();
}
