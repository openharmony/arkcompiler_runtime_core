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

function assertEq(a, b) {
	// console.log(`assertEq: '${a}' === '${b}'`)
	if (a !== b) {
		console.log(`assertEq failed: '${a}' === '${b}'`);
		process.exit(1);
	}
}

function runTest() {
	let etsVm = require(process.env.MODULE_PATH + '/ets_interop_js_napi.node');
	const etsOpts = {
		'panda-files': process.env.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
		'boot-panda-files': `${process.env.ARK_ETS_STDLIB_PATH}:${process.env.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH}`,
		'gc-trigger-type': 'heap-trigger',
		'load-runtimes': 'ets',
		'compiler-enable-jit': 'false',
		'enable-an': 'false',
		'run-gc-in-place': 'true',
	};
	const createRes = etsVm.createRuntime(etsOpts);
	if (!createRes) {
		console.log('Cannot create ETS runtime');
		process.exit(1);
	}

	testFunction(etsVm);
	testValues(etsVm);
}

function testFunction(etsVm) {
	const sumDouble = etsVm.getFunction('Lstatic_enum_test/ETSGLOBAL;', 'test1');
	assertEq(test1(), 0);
	const sumString = etsVm.getFunction('Lstatic_enum_test/ETSGLOBAL;', 'test2');
	assertEq(test2(), true);
}

function testValues(etsVm) {
	const Color = etsVm.getClass('Lstatic_enum_test/Color;');
	assertEq(Color.RED, 1);
	assertEq(Color.GREEN, 2);
	assertEq(Color.YELLOW, 3);
	assertEq(Color.BLACK, 4);
	assertEq(Color.BLUE, 5);

	const Direction = etsVm.getClass('Lstatic_enum_test/Direction;');

	assertEq(Direction.Up, 1);
	assertEq(Direction.Down, -1);
}

runTest();
