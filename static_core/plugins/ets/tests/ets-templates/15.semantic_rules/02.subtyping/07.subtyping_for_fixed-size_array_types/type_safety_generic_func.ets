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

function write<T>(arr: FixedArray<T>, v: T) {
    arr[0] = v;
}

function swap<T>(arr: FixedArray<T>, i: int, j: int) {
    let t = arr[i];
    arr[i] = arr[j];
    arr[j] = t;
}

function fill<T>(arr: FixedArray<T>, v: T) {
    for (let i = 0; i < arr.length; i++) arr[i] = v;
}

function set<T>(arr: FixedArray<T>, idx: int, v: T) {
    arr[idx] = v;
}

//for #4 test to escape smart typing (ca is effectivly FixedArray<D> after assigning)
//ca[0] = new C() could not be assigned
let da: FixedArray<D> = [new D(), new D()];
let ca: FixedArray<C> = da;

function main(): int {

    let testSuite = new arktest.ArkTestsuite("FixedArray_with_generics_and_subtyping");

    let exceptionCheck = (e: Error) => {
        return ( (e instanceof ArrayStoreError) && (e.message.includes(errMessage)) );
    }

    //#1 positive write check
    testSuite.addTest("function_write_positive", () => {
        let ca: FixedArray<C> = [new C()];
        write<C>(ca, new D())
    });

    //#2 Generic fn writes C into actual FixedArray<D>
    testSuite.addTest("function_write_negative", () => {
        arktest.expectThrow(
            () => {
                let da: FixedArray<D> = [new D()];
                let ca: FixedArray<C> = da;
                write<C>(ca, new C());
            },
            exceptionCheck
        );
    });

    //#3 positive swap check
    testSuite.addTest("function_swap_positive", () => {
        let ca: FixedArray<D> = [new D(), new D()];
        swap<C>(ca, 1, 0)
    });

    //#4 ArrayStoreError on direct write before generic call
    testSuite.addTest("function_swap_negative", () => {
        arktest.expectThrow(
            () => {
                ca[0] = new C();
                swap<C>(ca, 0, 1);
            },
            exceptionCheck
        );
    });

    //#5 positive fill check
    testSuite.addTest("function_fill_positive", () => {
        let ca: FixedArray<D> = [new D(), new D()];
        fill<C>(ca, new D());
    });

    //#6 number into actual FixedArray<string>
    testSuite.addTest("function_fill_negative", () => {
        arktest.expectThrow(
            () => {
                let sa: FixedArray<string> = ["a", "b"];
                let oa: FixedArray<Object> = sa;
                fill<Object>(oa, 42);
            },
            exceptionCheck
        );
    });

    //#7 positive set check
    testSuite.addTest("function_set_positive", () => {
        let ca: FixedArray<D> = [new D()];
        set<C>(ca, 0, new D());
    });

    //#8 String into FixedArray<Int> via Object
    testSuite.addTest("function_set_negative", () => {
        arktest.expectThrow(
            () => {
                let ia: FixedArray<Int> = [1, 2];
                let oa: FixedArray<Object> = ia;
                set<Object>(oa, 0, "wrong");
            },
            exceptionCheck
        );
    });

    return testSuite.run();
}
