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
 * @Import { jsVoid, returnAnonymous } from '../../shared/js/libFunctions.js'
 * @Tags interop, d2d
 */
<<<<<<< HEAD:static_core/plugins/ets/tests/benchmarks/interop_js/type_conversion_primitives/char/bu_a2a/bench_conversion_char_a2a.ets
class ConversionCharA2a {
    seed: number = 123;
    ch: string = 'A';
    result: string = '';
=======
function callImportFunction() {
>>>>>>> OpenHarmony_feature_20250328:static_core/plugins/ets/tests/benchmarks/interop_js/call_import_function/bu_d2d/call_import_function_j2j.js

    function callFunction(fun) {
        return fun(1000000, 1000000);
    };

    const anonymous = returnAnonymous();

    /**
     * @Benchmark
     */
<<<<<<< HEAD:static_core/plugins/ets/tests/benchmarks/interop_js/type_conversion_primitives/char/bu_a2a/bench_conversion_char_a2a.ets
    test(): string {
        this.result = this.charToNumberToString(this.ch);
        return this.result;
    }
=======
    this.callFun = function() {
        callFunction(jsVoid);
    };

    /**
     * @Benchmark
     */
    this.callAnonym = function() {
        callFunction(anonymous);
    };

>>>>>>> OpenHarmony_feature_20250328:static_core/plugins/ets/tests/benchmarks/interop_js/call_import_function/bu_d2d/call_import_function_j2j.js
}
