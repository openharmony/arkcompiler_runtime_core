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

let penv = process.env;
globalThis.etsVm = require(penv.MODULE_PATH + '/ets_interop_js_napi.node');

const etsVmRes = globalThis.etsVm.createRuntime({
	'log-level': 'error',
	'log-components': 'ets_interop_js',
	'boot-panda-files': penv.ARK_ETS_STDLIB_PATH + ':' + penv.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
	'panda-files': penv.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
	'gc-trigger-type': 'heap-trigger',
	'compiler-enable-jit': 'false',
	'run-gc-in-place': 'true',
});

if (!etsVmRes) {
	console.error(`Failed to create ETS runtime`);
	// eslint-disable-next-line
	return 1;
}

require('./demo');
