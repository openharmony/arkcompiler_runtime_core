/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

class ClassTest {
    a: int;

    constructor(a: int) {
        this.a = a;
    }
}

const etsArr: int[] = [];
const etsArrCopy: int[] = etsArr;
const etsArrOther: int[] = [];
const etsArrowFunction = () => { };
const etsArrowFunctionCopy = etsArrowFunction;
const etsArrowFunctionOther = () => { };
function foo() { }
function fooOther() { }

/**
 * @State
 * @Tags interop, bu_a2a
 */
class EqualityExtraA2a {
    noEqual: string = '==';
    equal: string = '!=';

    /**
     * @Setup
     */
    public setup(): void {
        console.log('Starting');
    }

    /**
     * @Benchmark
     */
    test(): number {
        //NOTE: issue(19194) enable below after fix compare arrays
        if (false) {
            this.comparison(etsArr, etsArr, this.equal, 'array ');
            this.comparison(etsArr, etsArrCopy, this.equal, 'array by link ');
        }

        this.comparison(etsArr, etsArrOther, this.noEqual, 'different arrays ');

        this.comparison(etsArrowFunction, etsArrowFunction, this.equal, 'arrow function ');

        this.comparison(etsArrowFunction, etsArrowFunctionCopy, this.equal, 'arrow function by link ');

        this.comparison(etsArrowFunction, etsArrowFunctionOther, this.noEqual, 'different arrows ');

        //NOTE: issue (19133) enable below after fix comparison functions
        if (false) {
            this.comparison(foo, foo, this.equal, 'function ');
            this.comparison(foo, fooOther, this.noEqual, 'different functions ');
        }

        return 1;
    }

    comparison<T>(valueA: T, valueB: T, compare: string, target: string): void {
        if (compare == this.equal && valueA != valueB) throw Error();
        if (compare == this.noEqual && valueA == valueB) throw Error();
    }
}
