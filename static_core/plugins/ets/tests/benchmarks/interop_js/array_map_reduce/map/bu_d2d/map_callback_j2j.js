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
 * @Include ./randomArray.js
 * @Import { addPrefix } from '../bu_s2d/libImport.ts'
 * @Tags interop, d2d
 */
function mapCallback() {

    const arrayLength = 100;
    const stringLength = 25;
    let testArray = [];
    let newTestArray = [];

    /**
     * @Setup
     */
    this.setup = function () {
<<<<<<< HEAD:static_core/plugins/ets/tests/benchmarks/interop_js/get_value/get_string_value/bu_j2a/bench_get_string_j2a.js
        console.log('Starting...');
        let stsVm = initEtsVm();

        this.getObj = stsVm.getFunction('Lbench_get_string_j2a/ETSGLOBAL;', 'getStsObj');

        return 0;
=======
        generateRandomArray(arrayLength, stringLength, testArray);
>>>>>>> OpenHarmony_feature_20250328:static_core/plugins/ets/tests/benchmarks/interop_js/array_map_reduce/map/bu_d2d/map_callback_j2j.js
    };

    /**
     * @Benchmark
     */
    this.test = function() {
        newTestArray = testArray.map(addPrefix);
    };

}
