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
function equalityStringJ2a() {
    const noEqual = '===';
    const equal = '!==';

    this.bench = null;

    /**
     * @Setup
     */
    this.setup = function () {
        console.log('Starting...');
        let stsVm = initEtsVm();

        const State = stsVm.getClass('Lbench_equality_string_j2a/EqualityStringJ2a;');

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
        comparison(this.bench.etsString, this.bench.etsString, equal, 'string ');
        return;
    };

    return;
}
