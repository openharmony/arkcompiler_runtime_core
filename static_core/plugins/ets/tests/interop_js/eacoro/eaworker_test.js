/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

async function runTest() {
    let etsVm = requireNapiPreview('ets_interop_js_napi', true);

    const etsVmRes = etsVm.createRuntime({
        'boot-panda-files': 'etsstdlib.abc:' + 'eaworker_test.abc',
        'gc-trigger-type': 'heap-trigger',
        'compiler-enable-jit': 'false',
        'run-gc-in-place': 'true',
        'coroutine-workers-count': '1',
    });
    if (!etsVmRes) {
        throw new Error('Failed to create ETS runtime');
    } else {
        print('ETS runtime created');
    }

    let RunTasksTest = etsVm.getFunction('Leaworker_test/ETSGLOBAL;', 'RunTasksTest');
    await RunTasksTest();
    let RunTasksWithJsCallTest = etsVm.getFunction('Leaworker_test/ETSGLOBAL;', 'RunTasksWithJsCallTest');
    await RunTasksWithJsCallTest();
    let CreateEAWorkerWithoutInterop = etsVm.getFunction('Leaworker_test/ETSGLOBAL;', 'CreateEAWorkerWithoutInterop');
    await CreateEAWorkerWithoutInterop();

    let RunSetTimeoutTest = etsVm.getFunction('Leaworker_test/ETSGLOBAL;', 'RunSetTimeoutTest');
    await RunSetTimeoutTest();
    let RunNewCoroInSetTimeoutTest = etsVm.getFunction('Leaworker_test/ETSGLOBAL;', 'RunNewCoroInSetTimeoutTest');
    await RunNewCoroInSetTimeoutTest();
}

runTest().catch((e) => {
    throw e;
});
