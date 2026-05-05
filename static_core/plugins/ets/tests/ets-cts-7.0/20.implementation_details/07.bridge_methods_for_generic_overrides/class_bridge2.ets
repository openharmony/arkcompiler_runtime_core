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
    ClassCastError in bridge methods for generic overrides with multiple parameters
---*/

class A {}
class B extends A {}

class Base<T extends A> {
    combine(a: T, b: T): T[] { return [a, b] }
}
class Derived extends Base<B> {
    override combine(a: B, b: B): B[] { return [a, b, a, b] }
}

class Base2<T extends Object, U extends A> {
    combine(a: T, b: U): Array<T|U> { return [a, b] }
}
class Derived2 extends Base2<A, B> {
    override combine(a: A, b: B): Array<A|B> { return [a, b, a, b] }
}

class Base3<T extends Object, U extends T> {
    combine(a: T, b: U): Array<T|U> { return [a, b] }
}
class Derived3 extends Base3<Numeric, Number> {
    override combine(a: Numeric, b: number): Array<Numeric|number> { return [a, b, b] }
}

let tmp: Any, a = new A, b = new B

function main(): int {
    let suite = new arktest.ArkTestsuite("ClassCastError in Bridge Methods");

    suite.addTest("Bridge methods expect B in both arguments", () => {
        tmp = new Derived();
        let d: Base<A> = tmp as Base<A>;
        arktest.assertEQ(d.combine(b, b).length, 4);
        arktest.expectThrow(
            () => { d.combine(a, a) },
            (e: Error) => e instanceof ClassCastError
        )
        arktest.expectThrow(
            () => { d.combine(a, b) },
            (e: Error) => e instanceof ClassCastError
        )
        arktest.expectThrow(
            () => { d.combine(b, a) },
            (e: Error) => e instanceof ClassCastError
        )
    });

    suite.addTest("Bridge methods expect A, B in arguments", () => {
        tmp = new Derived2();
        let d: Base2<Object, A> = tmp as Base2<Object, A>;
        arktest.assertEQ(d.combine(a, b).length, 4);
        arktest.expectThrow(
            () => { d.combine(new Object, b) },
            (e: Error) => e instanceof ClassCastError
        )
        arktest.expectThrow(
            () => { d.combine(a, a) },
            (e: Error) => e instanceof ClassCastError
        )
        arktest.expectThrow(
            () => { d.combine(new Object, a) },
            (e: Error) => e instanceof ClassCastError
        )
    });

    suite.addTest("Bridge methods expect Numeric, number in arguments", () => {
        tmp = new Derived3();
        let d: Base3<Object, number|float|boolean> = tmp as Base3<Object, number|float|boolean>;
        arktest.assertEQ(d.combine(1f, 1.0).length, 3);
        arktest.expectThrow(
            () => { d.combine(new Object, 1.0) },
            (e: Error) => e instanceof ClassCastError
        )
        arktest.expectThrow(
            () => { d.combine(1f, 1f) },
            (e: Error) => e instanceof ClassCastError
        )
        arktest.expectThrow(
            () => { d.combine(new Object, true) },
            (e: Error) => e instanceof ClassCastError
        )
    });

    return suite.run();
}
