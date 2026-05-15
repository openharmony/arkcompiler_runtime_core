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

function concat<T>(a: FixedArray<T>, b: FixedArray<T>): int {
    if ( (a[0] instanceof int) && (b[0] instanceof int) )
        return 0
    else return -1
}

interface Factory<T> {
    create(v: T): FixedArray<T>;
}
class IF implements Factory<int> {
    create(v: int): FixedArray<int> {
        return [v, v];
    }
}

function wrap<T extends Object>(v: T): FixedArray<int> {
    return [1];
}

function main(): int {

    let testSuite = new arktest.ArkTestsuite('FixedArray_with_generics_context');

    //#1 Generic fn for two FixedArrays
    testSuite.addTest('generic_func_positive', () => {
        let res = concat<string>(['a'], ['b']);
        arktest.assertEQ(res, -1);
    });

    //#2 Interface factory pattern with FixedArray return
    testSuite.addTest('fixed_array_factory_positive', () => {
        let f: Factory<int> = new IF();
        arktest.assertEQ(f.create(5)[0], 5);
    });

    //#3 Constrained T with array literal
    testSuite.addTest('constraint_positive', () => {
        let r = wrap<string>('x');
        arktest.assertEQ(r[0], 1);
    });

    return testSuite.run();
}
