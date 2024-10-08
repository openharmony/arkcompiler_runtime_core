/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

function init() {
	let etsVm = require(process.env.MODULE_PATH + '/ets_interop_js_napi.node');
	if (!etsVm.createEtsRuntime(process.env.ARK_ETS_STDLIB_PATH, process.env.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH, false, false)) {
		process.exit(1);
	}
	return etsVm;
}

function callEts(etsVm) {
	let res = etsVm.call('testReturnPendingPromise');
	if (typeof res !== 'object') {
		console.log('Result is not an object');
		process.exit(1);
	}
	if (res.constructor.name !== 'Promise') {
		console.log("Expect result type 'Promise' but get '" + res.constructor.name + "'");
		process.exit(1);
	}
	return res;
}

async function doAwait(p) {
	value = await p;
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
	queueMicrotask(() => {
		if (testSuccess) {
			console.log('Promise must not be resolved');
			process.exit(1);
		}
		if (!etsVm.call('resolvePendingPromise')) {
			console.log("Call of 'resolvePendingPromise' return false");
			return null;
		}
        queueMicrotask(() => {
            queueTasksHelper(testSuccess);
            return null;
        });
        return null;
	});
}

function queueTasksHelper(testSuccess) {
	if (!testSuccess) {
		console.log('Promise is not resolved or value is wrong');
		process.exit(1);
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
		console.log('No such test');
		process.exit(1);
	}
}

let args = process.argv;
if (args.length !== 3) {
	console.log('Expected test name');
	process.exit(1);
}
runTest(args[2]);
