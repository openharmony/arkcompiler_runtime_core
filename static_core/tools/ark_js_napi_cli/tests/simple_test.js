/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

function main() {
	globalThis.test = {};

	globalThis.test.etsVm = requireNapiPreview('ets_interop_js_napi_arkjsvm', true);

	const etsVmRes = globalThis.test.etsVm.createRuntime({
		'log-level': 'debug',
		'load-runtimes': 'ets',
		'log-components': 'ets_interop_js',
		'boot-panda-files': 'etsstdlib.abc:simple_test_sts.abc',
		'gc-trigger-type': 'heap-trigger',
		'compiler-enable-jit': 'false',
		'run-gc-in-place': 'true',
        'coroutine-workers-count': '1',
	});
	print('etsVm.version()=' + globalThis.test.etsVm.version());
	if (!etsVmRes) {
		throw new Error('Failed to create ETS runtime');
	} else {
		print('ETS runtime created');
	}
    let js_res = '';
    for (let j = 0; j < 10; ++j) {
        js_res += 'str_' + j + '\n';
    }

	let res = globalThis.test.etsVm.call('foo');
    print('Result:\n' + res);
    print(res === js_res);
}

main();
