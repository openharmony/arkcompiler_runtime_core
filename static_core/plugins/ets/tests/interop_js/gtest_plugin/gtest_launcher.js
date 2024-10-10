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

globalThis.ASSERT_TRUE = function assertTrue(v, msg = '') {
	if (v !== true) {
		msg = 'ASSERTION FAILED: ' + msg;
		throw Error(msg);
	}
};

globalThis.ASSERT_EQ = function assertEq(v0, v1) {
	if (!Object.is(v0, v1)) {
		let msg = `ASSERTION FAILED: ${v0}[${typeof v0}] !== ${v1}[${typeof v1}]`;
		throw Error(msg);
	}
};

globalThis.ASSERT_THROWS = function assertThrows(ctor, fn) {
	ASSERT_TRUE(ctor !== undefined && fn !== undefined);
	try {
		fn();
	} catch (exc) {
		if (exc instanceof ctor) {
			return;
		}
		throw Error('ASSERT_THROWS: expected instance of ' + ctor.name);
	}
	throw Error('ASSERT_THROWS: nothing was thrown');
};

globalThis.LOG_PROTO_CHAIN = function logProtoChain(o) {
	console.log('===== LOG_PROTO_CHAIN of ' + o + ' =====');
	for (let p = o.__proto__; p !== null; p = p.__proto__) {
		console.log(p.constructor + '\n[' + Object.getOwnPropertyNames(p) + ']');
	}
	console.log('==========');
};

function main() {
	// Add 'gtest' object to global space.
	// This object is used by gtests as storage to save and restore variables
	globalThis.gtest = {};
	globalThis.gtest.ret = 0;

	// load ets_interop_js_napi to globalThis.gtest.etsVm
	globalThis.gtest.etsVm = require('lib/module/ets_interop_js_napi');
	let penv = process.env;

	const etsVmRes = globalThis.gtest.etsVm.createRuntime({
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

	// 'globalThis.require' is used by gtests to load the node modules
	globalThis.require = require;

	let gtestName = process.argv[2];
	if (gtestName === undefined) {
		console.error(`Usage: ${process.argv[0]} ${process.argv[1]} <test name>`);
		return 1;
	}

	let gtestDir = process.env.ARR_ETS_INTEROP_JS_GTEST_DIR;
	if (gtestDir === undefined) {
		throw Error('ARR_ETS_INTEROP_JS_GTEST_DIR is not set');
	}

	// Run gtest
	console.log(`Run ets_interop_js_gtest module: ${gtestName}`);
	const etsGtest = require(`${gtestDir}/lib/module/${gtestName}`);
	let args = process.argv.slice(2);
	try {
		return etsGtest.main(args);
	} catch (e) {
		console.log(`${gtestName}: uncaught exception: ${e}`);
		console.log('exception.toString():\n', e.toString());
	}
	return 1;
}

process.exit(main());
