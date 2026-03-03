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
desc: Pass object literal without private field to Readonly type with different public field types
---*/

class A {
    public u: string = "a";
    public v: number = 55;
    private w: boolean = true;
}

function takeReadonly(arg: Readonly<A>): string {
    return arg.u;
}

function takePartialReadonly(arg: Partial<Readonly<A>>): string | undefined {
    return arg.u;
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Utility type private field - typed fields");

    suite.addTest("Pass object literal without private field to Readonly", () => {
        arktest.assertEQ(takeReadonly({ u: "b", v: 56 }), "b");
    });

    suite.addTest("Pass class instance to Readonly", () => {
        arktest.assertEQ(takeReadonly(new A()), "a");
    });

    suite.addTest("Pass partial object literal to Partial<Readonly>", () => {
        arktest.assertEQ(takePartialReadonly({ u: "f" }), "f");
    });

    return suite.run();
}
