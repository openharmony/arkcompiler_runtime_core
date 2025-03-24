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
function promiseJ2a() {
    this.promise = null;

    /**
     * @Setup
     */
    this.setup = function () {
        console.log('Starting...');
        let stsVm = initEtsVm();

        this.promise = stsVm.getFunction('Lbench_promise_j2a/ETSGLOBAL;', 'promise');

        return 0;
    };

    async function checkPromise() {
        await (() => {
            console.log(i);

            start = process.hrtime.bigint();
            return promise();
        }
        )().then(() => {
            loopTime = Number(process.hrtime.bigint() - start);
            return;
        });
        return;
    };

    /**
     * @Benchmark
     */
    this.test = function() {
        //NOTE issue(19656) enable this after fix global reference storage
        if (false) {
            await checkPromise();
        }
        return;
    };

    return;
}
