/**
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

function init() {
    let etsVm = require(process.env.MODULE_PATH + '/ets_interop_js_napi.node');

    let runtimeCreated = etsVm.createRuntime({
        'boot-panda-files': process.env.ARK_ETS_STDLIB_PATH + ':' + process.env.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH
    });

    if (!runtimeCreated) {
        process.exit(1);
    }
    return etsVm;
}

function runTest() {
    let etsVm = init();

    let waitForSchedule = () => {
        queueMicrotask(() => {
            let wasSchedulded = etsVm.call('.wasScheduled');
            if (wasSchedulded) {
                return;
            }
            setTimeout(waitForSchedule, 1);
        });
    };

    etsVm.call('.waitUntillJsIsReady');
    etsVm.call('.jsIsReady');
    setTimeout(waitForSchedule, 1);
}

runTest();
