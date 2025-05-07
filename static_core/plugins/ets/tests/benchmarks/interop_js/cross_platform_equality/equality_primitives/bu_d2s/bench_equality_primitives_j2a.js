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
 * @Tags interop, bu_d2s
 */
function equalityPrimitivesD2s() {
    const noEqual = '===';
    const equal = '!==';

    this.bench = null;

    /**
     * @Setup
     */
    this.setup = function () {
        let stsVm = initEtsVm();

        const State = stsVm.getClass('Lbench_equality_primitives_j2a/EqualityPrimitivesD2s;');

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
        return equal;
    };

    /**
     * @Benchmark
     * @returns {Obj}
     */
    this.test = function() {
        // NOTE issue (17741) - enable below after fix import bigInt
        if (false) {
            comparison(this.bench.etsBigInt, this.bench.etsBigInt, equal, 'BigInt ');
        }

        comparison(this.bench.etsBool, this.bench.etsBool, equal, 'boolean ');

        let result = comparison(this.bench.null, this.bench.null, equal, 'null ');

        // NOTE: (19193) enable below after fix comparison
        if (false) {
            comparison(this.bench.etsUndefined, this.bench.etsUndefined, equal, 'undefined ');
        }

        return result;
    };

    return;
}
