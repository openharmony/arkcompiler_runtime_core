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
 * @Include ../../../shared/js/initEtsVm.js
 * @Include ../bu_d2d/randomArray.js
 * @Tags interop, d2s
 */
function mapCallback() {

    this.addPrefix = null;
    const arrayLength = 100;
    const stringLength = 25;
    let testArray = [];
    let newTestArray = [];

    /**
     * @Setup
     */
    this.setup = function () {
        let stsVm = initEtsVm();
<<<<<<< HEAD:static_core/plugins/ets/tests/benchmarks/interop_js/call_import_function/bu_j2a/bench_call_import_function_j2a.js

        this.etsVoid = stsVm.getFunction('Lbench_call_import_function_j2a/ETSGLOBAL;', 'stsVoid');
        this.returnAnonymous = stsVm.getFunction('Lbench_call_import_function_j2a/ETSGLOBAL;', 'returnAnonymous');
        return 0;
    };

    function callFunction(fun, target) {
        fun(1000000, 1000000);
        return;
=======
        this.addPrefix = stsVm.getFunction('LlibImport/ETSGLOBAL;', 'addPrefix');
        generateRandomArray(arrayLength, stringLength, testArray);
>>>>>>> OpenHarmony_feature_20250328:static_core/plugins/ets/tests/benchmarks/interop_js/array_map_reduce/map/bu_d2s/map_callback_j2a.js
    };

    /**
     * @Benchmark
     */
    this.test = function() {
        // using arrow because callBackFn is actually a 'f(num, idx, arr)'
        newTestArray = testArray.map((x) => this.addPrefix(x));
    };

}
