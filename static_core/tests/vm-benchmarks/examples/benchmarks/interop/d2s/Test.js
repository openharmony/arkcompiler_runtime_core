/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
function Test() {

    this.bench = null;

    /**
     * @Setup
     */
    this.setup = function () {
        let stsVm = initEtsVm();
<<<<<<< HEAD:static_core/plugins/ets/tests/benchmarks/interop_js/stringify/array/bu_j2a/bench_stringify_array_j2a.js

        const getArray = stsVm.getFunction('Lbench_stringify_array_j2a/ETSGLOBAL;', 'getArray');

        this.array = getArray();

        return 0;
=======
        const State = stsVm.getClass('LTestImport/TestImport;');
        this.bench = new State();
        this.bench.setup();
>>>>>>> OpenHarmony_feature_20250328:static_core/tests/vm-benchmarks/examples/benchmarks/interop/d2s/Test.js
    };

    /**
     * @Benchmark
     * @returns {Int}
     */
    this.test = function() {
        return this.bench.testFunction();
    };

    return;
}
