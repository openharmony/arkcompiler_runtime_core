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
globalThis.require = require;

function runTest(test, warmup, iters) {
	console.log('Running test ' + test);
	let penv = process.env;
	let etsVm = require(penv.MODULE_PATH + '/ets_interop_js_napi');
	// NB: Consider setting compiler-enable-jit=false for local run
	const etsVmRes = etsVm.createRuntime({
		'log-level': 'info',
		'log-components': 'ets_interop_js',
		'boot-panda-files': penv.ARK_ETS_STDLIB_PATH + ':' + penv.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
		'panda-files': penv.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
		'gc-trigger-type': 'heap-trigger',
		'compiler-enable-jit': 'false',
		'run-gc-in-place': 'true',
	});

	if (!etsVmRes) {
		console.error('Failed to create ETS runtime');
		return 1;
	}

	globalThis.jsCode = require('./code.js');
	if (warmup > 0 && etsVm) {
		etsVm.call(test, warmup);
	}

	const MS2NS = 1000000;
	let start = Date.now();
	etsVm.call(test, iters);
	let timeNs = (Date.now() - start) * MS2NS;
	console.log('js interop test ' + test + ': ' + timeNs / iters + ' ns/iter, iters: ' + iters);

	return null;
}

let args = process.argv;
if (args.length !== 5) {
	console.log('Expected <test name> <number of warmup iterations> <number of iterations>');
	process.exit(1);
}
let test = args[2];
let warmup = parseInt(args[3], 10);
let iters = parseInt(args[4], 10);
runTest(test, warmup, iters);
