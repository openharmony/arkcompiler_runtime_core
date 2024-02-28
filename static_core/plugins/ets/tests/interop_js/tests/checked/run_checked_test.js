/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

function runTest(test, pandaOptions) {
	console.log('Running test ' + test);
	let penv = process.env;
	let etsVm = require(penv.MODULE_PATH + '/ets_interop_js_napi.node');
	globalThis.require = require;
	let options = [
		'--boot-panda-files',
		penv.ARK_ETS_STDLIB_PATH + ':' + penv.ARK_ETS_INTEROP_JS_TEST_ABC_PATH,
		'--panda-files',
		penv.ARK_ETS_INTEROP_JS_TEST_ABC_PATH,
	].concat(pandaOptions);
	const etsVmRes = etsVm.createRuntime(options);
	if (!etsVmRes) {
		console.error(`Failed to create ETS runtime`);
		return 1;
	}
	try {
		let res = etsVm.call(test);
		if (res !== 0) {
			throw 'test failed: ' + res;
		}
	} catch {
		return 1;
	}
	return 0;
}

let args = process.argv;

if (args.length < 4) {
	console.log('Expected ark options, test package and test function name');
	process.exit(1);
}
// checker.rb passes file and entry point as two last arguments
let test = args[args.length - 1];
runTest(test, args.slice(2, -2));
