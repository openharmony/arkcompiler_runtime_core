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
function getNumericJ2a() {

    this.getObj = null;

    /**
     * @Setup
     */
    this.setup = function () {
        console.log('Starting...');
        let penv = process.env;
        console.log('penv.MODULE_PATH: ' + penv.MODULE_PATH);
        let stsVm = require(penv.MODULE_PATH + '/ets_interop_js_napi.node');
        console.log(penv.ARK_ETS_STDLIB_PATH + ':' + penv.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH);
        const stsRT = stsVm.createRuntime({
            'boot-panda-files': penv.ARK_ETS_STDLIB_PATH + ':' + penv.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
            'panda-files': penv.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH
        });

        if (!stsRT) {
            console.error('Failed to create ETS runtime');
            return 1;
        }

        this.getObj = stsVm.getFunction('LETSGLOBAL;', 'getStsObj');

        return 0;
    };

    /**
     * @Benchmark
     */
    this.test = function() {
        const stsObj = this.getObj();

        const numVal = stsObj.stsNumber;
        const byteVal = stsObj.stsByte;
        const shortVal = stsObj.stsShort;
        const intVal = stsObj.stsInt;
        const longVal = stsObj.stsLong;
        const floatVal = stsObj.stsFloat;
        const doubleVal = stsObj.stsDouble;

        return;
    };

    return;
}
