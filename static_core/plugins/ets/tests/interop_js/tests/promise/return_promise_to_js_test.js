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

let iteration = 0;

function equal(actual, expected) {
	// try to convert expected to actual's type
	if (typeof actual === 'boolean') {
		return actual === Boolean(expected);
	} else if (typeof actual === 'number') {
		let eps = 1e-6;
		return Math.abs(actual - Number(expected)) < eps;
	}
	// convert all to string and compare string representation
	return String(actual) === String(expected);
}

function runTest(test, iter) {
	console.log("Running test '" + test + "'");
	let etsVm = require(process.env.MODULE_PATH + '/ets_interop_js_napi.node');
	if (!etsVm.createEtsRuntime(process.env.ARK_ETS_STDLIB_PATH, process.env.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH, false, false)) {
		console.log('Cannot create ETS runtime');
		process.exit(1);
	}
	let res = etsVm.call(test);
	if (typeof res !== 'object') {
		console.log('Result is not an object');
		process.exit(1);
	}
	if (res.constructor.name !== 'Promise') {
		console.log("Expect result type 'Promise' but get '" + res.constructor.name + "'");
		process.exit(1);
	}
	let testSuccess = false;
	res.then((value) => {
		if (equal(value, expectedValue)) {
			testSuccess = true;
		} else {
			console.log('Wrong value: ' + value + ' (' + typeof value + ') !== ' + expectedValue + ' (' + typeof expectedValue + ')');
		}
	});
	let callback = () => {
		--iteration;
		console.log('Checking #' + iteration);
		if (!testSuccess) {
			if (iteration > 0) {
				queueMicrotask(callback);
			} else {
				console.log('Promise is not resolved or value is wrong');
				process.exit(1);
			}
		} else {
			console.log('Test passed');
		}
	};
	queueMicrotask(callback);
}

let args = process.argv;
if (args.length !== 5) {
	console.log('Expected <test name> <expected value> <number of iterations>');
	process.exit(1);
}
iteration = args[4];
expectedValue = args[3];
runTest(args[2]);
