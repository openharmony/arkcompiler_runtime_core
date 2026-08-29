/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

const helper = requireNapiPreview('libinterop_test_helper.so', false);

function runTest(test) {
    const gtestAbcPath = helper.getEnvironmentVar('ARK_ETS_INTEROP_JS_GTEST_ABC_PATH');
    const stdlibPath = helper.getEnvironmentVar('ARK_ETS_STDLIB_PATH');
    const packageName = helper.getEnvironmentVar('PACKAGE_NAME');
	if (!packageName) {
		throw Error('PACKAGE_NAME is not set');
	}
	const globalName = 'L' + packageName + '/ETSGLOBAL;';

    let etsVm = requireNapiPreview('ets_interop_js_napi.so', false);

    let runtimeCreated = etsVm.createRuntime({
        'boot-panda-files': `${stdlibPath}:${gtestAbcPath}`,
        'panda-files': gtestAbcPath,
        'gc-trigger-type': 'heap-trigger',
        'compiler-enable-jit': 'false',
        'run-gc-in-place': 'true',
        'coroutine-workers-count': 2,
        'xgc-trigger-type': 'never',
        // 'log-debug': 'coroutines'
    });
    if (!runtimeCreated) {
        throw Error('Cannot create ETS runtime');
    }
    let valueToResolveWith = 42;
    const runTestImpl = etsVm.getFunction(globalName, test);
    const signalPromiseInJs = etsVm.getFunction(globalName, 'signalPromiseInJs');

    // Keep the libuv loop alive before starting the pending ETS Promise. The trigger is called
    // only after its JS Promise reaction runs on the completion coroutine.
    let failure = null;
    const triggerEventLoopCallback = helper.createEventLoopCallbackTrigger((stackInfoRestored) => {
        if (failure !== null) {
            throw failure;
        }
        if (!stackInfoRestored) {
            throw Error('ETS-to-JS scope did not restore the previous stack boundary');
        }
        print('Promise stack boundary restoration test passed');
    });

    let promise = runTestImpl(valueToResolveWith);
    if (promise == null) {
        throw Error('Function returned null');
    }
    promise.then((result) => {
        if (result !== valueToResolveWith) {
            failure = Error('Promise was not resolved correctly: result: ' + result +
                            ' expected: ' + valueToResolveWith);
        }
        // Do not invoke JS directly here: this reaction can still run inside napi_resolve_deferred().
        triggerEventLoopCallback();
    }, (error) => {
        failure = Error('Promise was rejected with error: ' + error);
        triggerEventLoopCallback();
    });
    signalPromiseInJs();
}

let args = helper.getArgv();
if (args.length !== 6) {
    throw Error('Expected test name');
}
runTest(args[5]);
