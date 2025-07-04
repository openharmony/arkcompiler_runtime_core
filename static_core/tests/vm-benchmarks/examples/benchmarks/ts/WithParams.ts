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
 */
<<<<<<< HEAD:static_core/plugins/ets/tests/benchmarks/interop_js/type_conversion_primitives/char/bu_a2j/bench_conversion_char_a2j.ets
class ConversionCharA2j {
    ch: string = "A";
    result: string = '';
=======
class WithParams {
>>>>>>> OpenHarmony_feature_20250328:static_core/tests/vm-benchmarks/examples/benchmarks/ts/WithParams.ts

    /**
     * @Param 1, 2
     */
    size: int;

    /**
     * @Param "a", "b"
     */
    value: string = "initial";

    /**
     * @Benchmark
     */
    test(): string {
<<<<<<< HEAD:static_core/plugins/ets/tests/benchmarks/interop_js/type_conversion_primitives/char/bu_a2j/bench_conversion_char_a2j.ets
        this.result = charToNumberToString(this.ch)
        return this.result;
=======
        return this.value + this.size.toString();
>>>>>>> OpenHarmony_feature_20250328:static_core/tests/vm-benchmarks/examples/benchmarks/ts/WithParams.ts
    }

}
