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

function cast<T>(obj: Object): FixedArray<T> {
    return obj as FixedArray<T>;
}

function identity<T>(arr: FixedArray<T>): FixedArray<T> {
    let obj: Object = arr;
    return obj as FixedArray<T>;
}

function reinterpret<T, U>(arr: FixedArray<T>): FixedArray<U> {
    return arr as Object as FixedArray<U>;
}

class Wrapper<T> {
    unwrap(obj: Object): FixedArray<T> {
        return obj as FixedArray<T>;
    }
}

class A<T> {
    field: FixedArray<T>;
    constructor(v: FixedArray<T>) {
        this.field = v
    }
}

function main(): int {

    let testSuite = new arktest.ArkTestsuite("FixedArray_with_generics_cast");

    let exceptionCheckCast = (e: Error) => {
        return ( (e instanceof ClassCastError) && (e.message.includes('cannot be cast to')) );
    }

    //#1 Generic cast function, wrong type arg
    testSuite.addTest("cast_function_negative", () => {
        arktest.expectThrow(
            () => {
                let sa: FixedArray<string> = ["a"];
                let obj: Object = sa;
                cast<int>(obj);
            },
            exceptionCheckCast
        );
    });

    //#2 Generic class method cast
    testSuite.addTest("cast_class_negative", () => {
        arktest.expectThrow(
            () => {
                let w = new Wrapper<int>();
                let sa: FixedArray<string> = ["a"];
                w.unwrap(sa as Object);
            },
            exceptionCheckCast
        );
    });

    //#3 same type roundtrip
    testSuite.addTest("cast_function_arg_positive", () => {
        let ia: FixedArray<int> = [1];
        identity<int>(ia);
    });

    //#4 Two-type-param reinterpret cast
    testSuite.addTest("cast_Two-type-param_function_negative", () => {
        arktest.expectThrow(
            () => {
                let sa: FixedArray<string> = ["a"];
                reinterpret<string, int>(sa);
            },
            exceptionCheckCast
        );
    });

    //#5 access FixedArray field (A erases, but field access triggers check on FixedArray which preserves type)
    testSuite.addTest("cast_field_access_negative", () => {
        arktest.expectThrow(
            () => {
                let a = new A<string>(["a"]);
                let obj: Object = a;
                let b = obj as A<int>;
                b.field;
            },
            exceptionCheckCast
        );
    });

    return testSuite.run();
}
