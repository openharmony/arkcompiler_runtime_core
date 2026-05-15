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
    Fixed-size array types are covariant and are formally defined as follows:
    FixedSize<B> <: FixedSize<A> if B <: A.
    Fixed-size array can be created by using Array Literal or constructors defined for
    type FixedArray<T>, where T must be a type preserved by Type Erasure.
---*/

abstract class Repo<T> {
    abstract getAll(): FixedArray<T>;
    abstract save(v: T): void;
}
class StringRepo extends Repo<string> {
    private data: FixedArray<string> = ["a"];
    override getAll(): FixedArray<string> {
        return this.data;
    }
    override save(v: string) {
        this.data[0] = v;
    }
}

abstract class Base<T> {
    abstract process(arr: FixedArray<T>): void;
}
class Derived extends Base<int> {
    override process(arr: FixedArray<int>) {
        arr[0] = 99;
    }
}

function main(): int {

    let testSuite = new arktest.ArkTestsuite("FixedArray_with_generics_override");

    let exceptionCheckCast = (e: Error) => {
        return ( (e instanceof ClassCastError) && (e.message.includes('cannot be cast to')) );
    }

    //#1
    testSuite.addTest("override_return_positive", () => {
        let r: Repo<string> = new StringRepo();
        r.save("b");
        arktest.assertEQ(r.getAll()[0], "b");
    });

    //#2
    testSuite.addTest("override_argument_positive", () => {
        let b: Base<int> = new Derived();
        let a: FixedArray<int> = [1];
        b.process(a);
        arktest.assertEQ(a[0], 99);
    });

    //#3
    testSuite.addTest("constraint_negative", () => {
        arktest.expectThrow(
            () => {
                let b: Base<int> = new Derived();
                let a: FixedArray<Object> = [new Object()];
                let c = b as Base<Object>
                c.process(a);
            },
            exceptionCheckCast
        );
    });

    return testSuite.run();
}
