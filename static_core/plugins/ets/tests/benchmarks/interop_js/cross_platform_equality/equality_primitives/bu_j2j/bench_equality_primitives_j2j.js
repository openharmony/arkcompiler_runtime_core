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

const {
    jsBigInt,
    jsBool,
    jsNull,
    jsUndefined,
} = require('./import');

/**
 * @State
 * @Tags interop, bu_j2j
 */
function equalityPrimitivesJ2j() {
    const noEqual = '===';
    const equal = '!==';

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
     * @Setup
     */
    this.setup = function () {
        console.log('Starting...');
        return;
    };

    /**
     * @Benchmark
     */
    this.test = function() {
        // NOTE issue (17741) - enable below after fix import bigInt
        if (false) {
            comparison(jsBigInt, jsBigInt, equal, 'BigInt ');
        }

        comparison(jsBool, jsBool, equal, 'boolean ');

        comparison(jsNull, jsNull, equal, 'null ');

        // NOTE: (19193) enable below after fix comparison
        if (false) {
            comparison(jsUndefined, jsUndefined, equal, 'undefined ');
        }

        return;
    };

    return;
}


