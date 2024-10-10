/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

async function runTest(test) {
    let etsVm = require(process.env.MODULE_PATH + '/ets_interop_js_napi.node');

	let runtimeCreated = etsVm.createRuntime({
		'boot-panda-files': process.env.ARK_ETS_STDLIB_PATH + ':' + process.env.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
		'panda-files': process.env.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
		'gc-trigger-type': 'heap-trigger',
		'compiler-enable-jit': 'false',
		'run-gc-in-place': 'true',
		'coroutine-impl': 'stackful',
		'coroutine-workers-count': 2,
		'coroutine-js-mode': true,

	});
    if (!runtimeCreated) {
        console.log('Cannot create ETS runtime');
        process.exit(1);
    }
    let valueToResolveWith = 42;
    let promise = etsVm.call(test, valueToResolveWith);
    if (promise == null) {
        console.log('Function returned null');
        process.exit(-1);
    }
    etsVm.call('signalPromiseInJs');
    try {
        let result = await promise;
        if (result !== valueToResolveWith) {
            console.log('Promise was not resolved correctly: result: ', result, ' expected: ', valueToResolveWith);
            process.exit(-1);
        }
    } catch (error) {
        console.log('Promise was rejected with error:', error);
        process.exit(-1);
    }
}

let args = process.argv;
if (args.length !== 3) {
    console.log('Expected test name');
    process.exit(1);
}
runTest(args[2]);