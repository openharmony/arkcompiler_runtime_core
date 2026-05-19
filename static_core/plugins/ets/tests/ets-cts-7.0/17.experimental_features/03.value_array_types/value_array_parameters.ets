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
desc: |-
    Value array type is the built-in type written as ValueArray<T> and characterized by the fact:
    Any instance of array type contains elements of type T. T must be a value type
    Test assertions:
    - Any instance of array type contains elements of type T. T must be a value type
    - The number of elements is known as array length, and can be accessed by using the property length.
    - Array element is accessed by its index. Index is an integer number starting from 0 to array length minus 1.
    - Type of each array element is equal to the element(s) type specified in the array declaration.
    - Value array can be created by using Array Literal or constructors defined for type ValueArray<T>
    - Value assigning
    ValueArray<T> is not a generic type, despite using notation identical to generics.
---*/

function returnFunc(): ValueArray<int> {
    return [1]
}

function paramFunc(p: ValueArray<int>|undefined)  {
    return p?.[0]
}

function genFunc<T> (v: T, arr: ValueArray<long>): T {
    return (arr.length + 1.5) as T
}

interface I {
    foo(): ValueArray<number> {
        return [-1.1, 0, 99.89]
    }
    bar(p: ValueArray<char>): int
}

class B {
    foo(p: ValueArray<float>): ValueArray<long> {
        return [333]
    }
}

class A extends B implements I {
    override foo(): ValueArray<number> {
        return [1, 20000000001.1]
    }

    bar(p: ValueArray<char>): int {
        return 0
    }
}

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Value_array_parameters");

    //#1 ValueArray function return check
    testSuite.addTest('function_return', () => {
        arktest.assertEQ( returnFunc()[0], 1 )
    });

    //#2 ValueArray parameters check
    testSuite.addTest('function_as_parameter', () => {
        let arr: ValueArray<int> = [9, 0]
        arktest.assertEQ( paramFunc(arr), 9 )
        arktest.assertEQ( paramFunc(undefined), undefined )
    });

    //#3 ValueArray generic function
    testSuite.addTest('generic_function', () => {
        // array length 3 + 1.5
        arktest.assertEQ( genFunc<double>(new double, [7, 1, 7]) , 4.5 )
    });

    //#4 ValueArray class methods
    testSuite.addTest('class_methods', () => {
       let test: I = new A()
       //class methods
       arktest.assertEQ( test.foo().length, 2 )
       arktest.assertEQ( test.bar([c'o']), 0 )
    });

    return testSuite.run();
}
