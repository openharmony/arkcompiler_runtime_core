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

import {
    jsArr,
    jsArrCopy,
    jsArrOther,
    jsArrowFoo,
    jsArrowFooCopy,
    jsArrowFooOther,
    jsFoo,
    jsFooOther,
} from 'import';

/**
 * @State
 * @Tags interop, bu_a2j
 */
class EqualityExtraA2j {
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
            this.comparisonObject(jsArr, jsArr, this.equal, 'array ');
            this.comparisonObject(jsArr, jsArrCopy, this.equal, 'array by link ');
        }

        this.comparisonObject(jsArr, jsArrOther, this.noEqual, 'different arrays ');

        this.comparisonObject(jsArrowFoo, jsArrowFoo, this.equal, 'arrow function ');

        this.comparisonObject(jsArrowFoo, jsArrowFooCopy, this.equal, 'arrow function by link ');

        this.comparisonObject(jsArrowFoo, jsArrowFooOther, this.noEqual, 'different arrow functions ');

        //NOTE: issue (19133) enable below after fix comparison functions
        if (false) {
            this.comparisonObject(jsFoo, jsFoo, this.equal, 'function ');
            this.comparisonObject(jsFoo, jsFooOther, this.noEqual, 'different functions ');
        }

        return 1;
    }

    comparisonObject(valueA: JSValue, valueB: JSValue, compare: string, target: string): void {
        if (compare == this.equal && valueA !== valueB) throw Error();
        if (compare == this.noEqual && valueA === valueB) throw Error();
    }

    comparison<T>(valueA: T, valueB: T, compare: string, target: string): void {
        if (compare == this.equal && valueA != valueB) throw Error();
        if (compare == this.noEqual && valueA == valueB) throw Error();
    }
}
