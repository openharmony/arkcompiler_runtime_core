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
 * @Include ../../shared/js/initEtsVm.js
 * @Tags interop, d2s
 */
function testStringify() {
    this.array = null;

    /**
     * @Setup
     */
    this.setup = function () {
<<<<<<< HEAD:static_core/plugins/ets/tests/benchmarks/interop_js/get_value/get_specific_value/bu_j2a/bench_get_specific_j2a.js
        console.log('Starting...');
        let stsVm = initEtsVm();

        this.getObj = stsVm.getFunction('Lbench_get_specific_j2a/ETSGLOBAL;', 'getStsObj');

        return 0;
=======
        let etsVm = initEtsVm();
        this.holder = etsVm.getClass('LlibArrayHolder/ArrayHolder;');
>>>>>>> OpenHarmony_feature_20250328:static_core/plugins/ets/tests/benchmarks/interop_js/stringify/bu_d2s/stringify_j2a.js
    };

    /**
     * @Benchmark
     * @returns {Obj}
     */
    this.testArray = function() {
        return JSON.stringify(this.holder.arr);
    };

    /**
     * @Benchmark
     * @returns {Obj}
     */
    this.testObject = function() {
        return JSON.stringify(this.holder.obj);
    };

}
