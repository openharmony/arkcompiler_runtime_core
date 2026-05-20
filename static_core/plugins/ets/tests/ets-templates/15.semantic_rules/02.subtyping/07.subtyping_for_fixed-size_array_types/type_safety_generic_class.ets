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

const errMessage = 'cannot be stored in an array of type'

class C {}
class D extends C {}

class Container<T> {
    arr: FixedArray<T>;
    constructor(a: FixedArray<T>) {
        this.arr = a;
    }
    set(i: int, v: T) {
        this.arr[i] = v;
    }
}

class Pair<T> {
    first: FixedArray<T>;
    second: FixedArray<T>;
    setFirst(v: T) {
        this.first[0] = v;
    }
    setSecond(v: T) {
        this.second[0] = v;
    }
    constructor(a: FixedArray<T>, b: FixedArray<T>) {
        this.first = a;
        this.second = b;
    }
}

class Stack<T> {
    data: FixedArray<T>;
    push(v: T) {
        this.data[this.data.length - 1] = v;
    }
    constructor(a: FixedArray<T>) {
        this.data = a;
    }
}


function main(): int {

    let testSuite = new arktest.ArkTestsuite("FixedArray_with_generics_and_subtyping_class");

    let exceptionCheck = (e: Error) => {
        return ( (e instanceof ArrayStoreError) && (e.message.includes(errMessage)) );
    }

    //#1 D is subtype of C - no error (container)
    testSuite.addTest("class_container_positive", () => {
        let c = new Container<C>([new C]);
        c.set(0, new D());
    });

    //#2 Write through generic class method, actual array is FixedArray<D>
    testSuite.addTest("class_container_negative", () => {
        arktest.expectThrow(
            () => {
                let d = new Container<D>([new D()]);
                let c: Container<C> = d as Container<C>;
                c.set(0, new C());
            },
            exceptionCheck
        );
    });

    //#3 D is subtype of C - no error (multi_field)
    testSuite.addTest("multi_field_class_positive", () => {
        let dp = new Pair<C>([new C], [new C]);
        dp.setFirst(new D())
    });

    //#4 Multi-field generic class
    testSuite.addTest("multi_field_class_negative", () => {
        arktest.expectThrow(
            () => {
                let dp = new Pair<D>([new D()], [new D()]);
                let cp: Pair<C> = dp as Pair<C>;
                cp.setFirst(new C())
            },
            exceptionCheck
        );
    });

    //#5 D is subtype of C - no error (Stack)
    testSuite.addTest("stack_class_positive", () => {
        let cp = new Stack<C>([new C()]);
        cp.push(new D());
    });

    //#6 Stack-like pattern
    testSuite.addTest("stack_class_negative", () => {
        arktest.expectThrow(
            () => {
                let ds = new Stack<D>([new D()]);
                let cs: Stack<C> = ds as Stack<C>;
                cs.push(new C());
            },
            exceptionCheck
        );
    });

    return testSuite.run();
}
