/**
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
 * @Tags interop, bu_j2j
 */
function conversionOctalJ2j() {
    const seed = 123;
    let octalString = '';
    let result;

    function generateNumber(seed) {
        const modulus = Math.pow(2, 32);
        const a = 1664525;
        const c = 1013904223;
      
        seed = (a * seed + c) % modulus;
        
        return Math.floor((seed / modulus) * 100); 
    };

    function octalStringToNumberToString(str) {
        const res = parseInt(str, 8);
        return String(res);
    };

    const data = generateNumber(seed);
    octalString = data.toString(8);

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
        result = octalStringToNumberToString(octalString);
        return;
    };

    return;
}
