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

function runTest() {
	let test = 'toEtsAndBack';
	console.log('Running test ' + test);
	let etsVm = require(process.env.MODULE_PATH + '/ets_interop_js_napi.node');
	if (!etsVm.createEtsRuntime(process.env.ARK_ETS_STDLIB_PATH, process.env.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH, false, false)) {
		console.log('Cannot create ETS runtime');
		process.exit(1);
	}
	let jsPromise = Promise.resolve();
	let etsPromise = etsVm.call('getTheSamePromise', jsPromise);
	if (jsPromise !== etsPromise) {
		console.log('Test ' + test + ' failed: expected jsPromise and etsPromise are the same but actually they differs');
		process.exit(1);
	}
}

runTest();
