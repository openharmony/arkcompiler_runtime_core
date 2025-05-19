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
function getStringJ2a() {
    this.getObj = null;

    /**
     * @Setup
     */
    this.setup = function () {
        console.log('Starting...');
        let stsVm = initEtsVm();

        this.getObj = stsVm.getFunction('Lbench_get_string_j2a/ETSGLOBAL;', 'getStsObj');

        return 0;
    };

    /**
     * @Benchmark
     */
    this.test = function() {
        const stsObj = this.getObj();

        const stringVal = stsObj.etsString;

        return;
    };

    return;
}
