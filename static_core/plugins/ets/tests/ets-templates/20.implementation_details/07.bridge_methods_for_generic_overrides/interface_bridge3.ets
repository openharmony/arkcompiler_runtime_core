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
    ClassCastError in bridge methods for generic overrides with array types
---*/

interface Base<T> {
    process(p: T[]): T
}
class Derived implements Base<number> {
    override process(p: number[]): number { return p[0] * 2 }
}

interface Base2<T> {
    process(p: FixedArray<T>): T
}
class Derived2 implements Base2<number> {
    override process(p: FixedArray<number>): number { return p[0] * 2 }
}

interface Base3<T> {
    process(p: T): T
}
class Derived3 implements Base3<number[]> {
    override process(p: Array<number>): number[] { return p }
}

interface Base4<T extends Object> {
    process(p: T): T
}
class Derived4 implements Base4<FixedArray<number>> {
    override process(p: FixedArray<number>): FixedArray<number> { return p }
}

let tmp: Any

function main(): int {
    let suite = new arktest.ArkTestsuite("ClassCastError in Bridge Methods");

    suite.addTest("No bridge method created here, CCE is thrown from the overridden method", () => {
        tmp = new Derived();
        let d: Base<Object> = tmp as Base<Object>;
        arktest.assertEQ(d.process([1.0]), 2.0);
        arktest.expectError(
            () => { d.process([new Object]) },
            "std.core.Object cannot be cast to std.core.Double"
        )
    });

    suite.addTest("Bridge method expects FixedArray<number>", () => {
        tmp = new Derived2();
        let d: Base2<Object> = tmp as Base2<Object>;
        arktest.assertEQ(d.process([1.0] as FixedArray<number>), 2.0);
        arktest.expectError(
            () => { d.process([new Object] as FixedArray<Object>) },
            "std.core.Object[] cannot be cast to std.core.Double[]"
        )
    });

    suite.addTest("Bridge method expects Array", () => {
        tmp = new Derived3();
        let d: Base3<Object> = tmp as Base3<Object>;
        arktest.assertEQ((d.process([1.0]) as number[]).length, 1);
        arktest.assertEQ((d.process(["a", "b"]) as string[]).length, 2);
        arktest.expectError(
            () => { d.process(new Object) },
            "std.core.Object cannot be cast to std.core.Array"
        )
    });

    suite.addTest("Bridge method expects FixedArray<number>", () => {
        tmp = new Derived4();
        let d: Base4<Object> = tmp as Base4<Object>;
        arktest.assertEQ((d.process([1.0] as FixedArray<number>) as FixedArray<number>).length, 1);
        arktest.expectError(
            () => { d.process([new Object] as FixedArray<Object>) },
            "std.core.Object[] cannot be cast to std.core.Double[]"
        )
    });

    return suite.run();
}
