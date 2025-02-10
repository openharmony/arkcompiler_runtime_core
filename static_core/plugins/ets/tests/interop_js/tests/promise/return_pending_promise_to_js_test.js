/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

let testSuccess = false;

const helper = requireNapiPreview('libinterop_test_helper.so', false);

function init() {
	const gtestAbcPath = helper.getEnvironmentVar('ARK_ETS_INTEROP_JS_GTEST_ABC_PATH');
	const stdlibPath = helper.getEnvironmentVar('ARK_ETS_STDLIB_PATH');

	let etsVm = requireNapiPreview('ets_interop_js_napi_arkjsvm.so', false);

	const etsOpts = {
		'panda-files': gtestAbcPath,
		'boot-panda-files': `${stdlibPath}:${gtestAbcPath}`,
		'coroutine-js-mode': true,
		'coroutine-enable-external-scheduling': 'true',
	};
	if (!etsVm.createRuntime(etsOpts)) {
		throw Error('Cannot create ETS runtime');
	}
	return etsVm;
}

function callEts(etsVm) {
	let res = etsVm.call('.testReturnPendingPromise');
	if (typeof res !== 'object') {
		throw Error('Result is not an object');
	}
	if (res.constructor.name !== 'Promise') {
		throw Error("Expect result type 'Promise' but get '" + res.constructor.name + "'");
	}
	return res;
}

async function doAwait(p) {
	let value = await p;
	if (value === 'Panda') {
		testSuccess = true;
	}
}

function doThen(p) {
	p.then((value) => {
		if (value === 'Panda') {
			testSuccess = true;
		}
	});
}

function queueTasks(etsVm) {
	helper.setTimeout(() => {
		if (testSuccess) {
			throw Error('Promise must not be resolved');
		}
		if (!etsVm.call('.resolvePendingPromise')) {
			throw Error("Call of 'resolvePendingPromise' return false");
		}
        helper.setTimeout(queueTasksHelper, 0, testSuccess);
	}, 0);
}

function queueTasksHelper(testSuccess) {
	if (!testSuccess) {
		throw Error('Promise is not resolved or value is wrong');
	}
	return null;
}

function runAwaitTest() {
	let etsVm = init();
	let res = callEts(etsVm);
	doAwait(res);
	queueTasks(etsVm);
}

function runThenTest() {
	let etsVm = init();
	let res = callEts(etsVm);
	doThen(res);
	queueTasks(etsVm);
}

function runTest(test) {
	if (test === 'await') {
		runAwaitTest();
	} else if (test === 'then') {
		runThenTest();
	} else {
		throw Error('No such test');
	}
}

let args = helper.getArgv();
if (args.length !== 5) {
	throw Error('Expected test name');
}
runTest(args[4]);
