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
desc: Test passing an object literal without private field to a function expecting Readonly<A>
---*/

class A {
    public x = 1;
    private y = 2;
}

function takeReadonly(arg: Readonly<A>): int {
    return arg.x;
}

function takePartialReadonly(arg: Partial<Readonly<A>>): int | undefined {
    return arg.x;
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Utility type private field - simple class");

    suite.addTest("Pass object literal without private field to Readonly", () => {
        arktest.assertEQ(takeReadonly({ x: 3 }), 3);
    });

    suite.addTest("Pass class instance to Readonly", () => {
        arktest.assertEQ(takeReadonly(new A()), 1);
    });

    suite.addTest("Pass object literal without private field to Partial<Readonly>", () => {
        arktest.assertEQ(takePartialReadonly({ x: 4 }), 4);
    });

    suite.addTest("Assign Readonly variable to another Readonly variable", () => {
        let obj1: Readonly<A> = new A();
        let obj2: Readonly<A> = obj1;
        arktest.assertEQ(obj2.x, 1);
    });

    return suite.run();
}
