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
    jsFooOther } from './test_import';

/**
 * @State
 * @Tags interop, bu_d2d
 */
function equalityExtraD2d() {
    const noEqual = '===';
    const equal = '!==';

    function comparison(valueA, valueB, compare, target) {
        if (compare === equal && valueA !== valueB) {
            throw Error();
        }
        if (compare === noEqual && valueA === valueB) {
            throw Error();
        }
        return equal;
    };

    /**
     * @Benchmark
     * @returns {Obj}
     */
    this.test = function() {
        //NOTE: issue(19194) enable below after fix compare arrays
        if (false) {
            comparison(jsArr, jsArr, equal, 'array ');
            comparison(jsArr, jsArrCopy, equal, 'array by link ');
        }

        comparison(jsArr, jsArrOther, noEqual, 'array different array ');

        comparison(jsArrowFoo, jsArrowFoo, equal, 'arrow function ');

        comparison(jsArrowFoo, jsArrowFooCopy, equal, 'arrow function by link ');

        let result = comparison(jsArrowFoo, jsArrowFooOther, noEqual, 'arrow different arrow ');

        //NOTE: issue (19133) enable below after fix comparison functions
        if (false) {
            comparison(jsFoo, jsFoo, equal, 'function with itself ');
            comparison(jsFoo, jsFooOther, noEqual, 'function different function ');
        }

        return result;
    };

    return;
}
