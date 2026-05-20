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

class Base {}
class Mid extends Base {}
class Leaf extends Mid {}

class D1 extends Base {}
class D2 extends Base {}
function process<T extends Base>(arr: FixedArray<T>, v: T) {
    arr[0] = v;
}

interface ComparableI {
    compareTo(other: Object): int;
}
class MyStr implements ComparableI {
    compareTo(other: Object): int {
        //compare stub
        return -1;
    }
}
function sort<T extends ComparableI>(arr: FixedArray<T>) {
    //sort stub
}

type CustomArr<T extends Base> = FixedArray<T>;

//for #3 test to escape smart typing
let ca: CustomArr<Leaf> = [new Leaf()];
let ba: FixedArray<Base> = ca;
//for #4 test to escape smart typing
let d1arr: FixedArray<D1> = [new D1()];
let barr: FixedArray<Base> = d1arr;

function main(): int {

    let testSuite = new arktest.ArkTestsuite("FixedArray_with_generics_constraints");

    let exceptionCheck = (e: Error) => {
        return ( (e instanceof ArrayStoreError) && (e.message.includes('cannot be stored in an array of type')) );
    }

    //#1 Constrained T, write Base into actual FixedArray
    testSuite.addTest("constraint_negative", () => {
        arktest.expectThrow(
            () => {
                let la: FixedArray<Leaf> = [new Leaf()];
                let ba: FixedArray<Base> = la;
                process<Base>(ba, new Base());
            },
            exceptionCheck
        );
    });

    //#2 Positive: correct constrained type
    testSuite.addTest("constraint_positive", () => {
        let arr: FixedArray<MyStr> = [];
        sort<MyStr>(arr);
    });

    //#3 Type alias with constraint, wrong mid-hierarchy type
    testSuite.addTest("mid-hierarchy_type_negative", () => {
        arktest.expectThrow(
            () => {
                ba[0] = new Mid();
            },
            exceptionCheck
        );
    });

    //#4 Sibling class via covariant base
    testSuite.addTest("sibling_negative", () => {
        arktest.expectThrow(
            () => {
                barr[0] = new D2();
            },
            exceptionCheck
        );
    });

    return testSuite.run();
}
