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

/**
 * @State
 * @Tags interop, bu_j2a
 */
function equalityExtraJ2a() {
    const noEqual = '===';
    const equal = '!==';

    this.bench = null;
    this.foo = null;
    this.fooOther = null;

    /**
     * @Setup
     */
    this.setup = function () {
        console.log('Starting...');
        let stsVm = initEtsVm();

        const State = stsVm.getClass('Lbench_equality_extra_j2a/EqualityExtraJ2a;');

        this.foo = stsVm.getFunction('Lbench_equality_extra_j2a/ETSGLOBAL;', 'foo');
        this.fooOther = stsVm.getFunction('Lbench_equality_extra_j2a/ETSGLOBAL;', 'fooOther');

        this.bench = new State();

        return 0;
    };

    function comparison(valueA, valueB, compare, target) {
        if (compare === equal && valueA !== valueB) {
            throw Error();
        }
        if (compare === noEqual && valueA === valueB) {
            throw Error();
        }
        return;
    };

    /**
     * @Benchmark
     */
    this.test = function() {
        //NOTE: issue(19194) enable below after fix compare arrays
        if (false) {
            comparison(this.bench.etsArr, this.bench.etsArr, equal, 'array ');
            comparison(this.bench.etsArr, this.bench.etsArrCopy, equal, 'array by link ');
        }

        comparison(this.bench.etsArr, this.bench.etsArrOther, noEqual, 'array different array ');

        comparison(this.bench.etsArrowFunction, this.bench.etsArrowFunction, equal, 'arrow function ');

        comparison(this.bench.etsArrowFunction, this.bench.etsArrowFunctionCopy, equal, 'arrow function by link ');

        comparison(this.bench.etsArrowFunction, this.bench.etsArrowFunctionOther, noEqual, 'arrow different arrow ');

        //NOTE: issue (19133) enable below after fix comparison functions
        if (false) {
            comparison(this.foo, this.foo, equal, 'function with itself ');
            comparison(this.foo, this.fooOther, noEqual, 'function different function ');
        }

        return;
    };

    return;
}
