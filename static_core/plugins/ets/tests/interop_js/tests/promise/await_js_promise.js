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
// resolver: (value) => {}
let resolver = undefined;

let PROMISE_RESOLVE_VALUE = 'JS_PROMISE_RESULT';

// let LOG_LEVEL = 5
let LOG_LEVEL = 1;
let FATAL = 0;
let ERROR = 1;
let INFO = 2;
function msg(s, lvl) {
	if (LOG_LEVEL >= lvl) {
		console.log(s);
	}
}

function init() {
	let etsVm = require(process.env.MODULE_PATH + '/ets_interop_js_napi');
	const etsOpts = {
		'panda-files': process.env.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
		'boot-panda-files': `${process.env.ARK_ETS_STDLIB_PATH}:${process.env.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH}`,
		'gc-trigger-type': 'heap-trigger',
		'load-runtimes': 'ets',
		'compiler-enable-jit': 'false',
		'run-gc-in-place': 'true',
		'coroutine-js-mode': 'true',
		'coroutine-impl': 'stackful',
		//'log-debug': 'coroutines'
	};
	const createRes = etsVm.createRuntime(etsOpts);
	if (!createRes) {
		console.log('Cannot create ETS runtime');
		process.exit(1);
	}
	return etsVm;
}

async function doAwait(pr) {
	msg('Await func: start', INFO);
	value = await pr;
	msg('Await func: await returned: ' + value, INFO);
	if (value === 'Panda') {
		testSuccess = true;
		msg('Await func: value from await is correct!', INFO);
	}
	msg('Await func: end', INFO);
}

function createPendingPromise() {
	let p = new Promise((res, rej) => {
		resolver = res;
	});
	return p;
}

function createResolvedPromise() {
	let p = new Promise((res, rej) => {
		res(PROMISE_RESOLVE_VALUE);
	});
	return p;
}

function runAwaitTest(name, promiseCreator, promiseResolver) {
	msg('Running test ' + name, INFO);
	let etsVm = init();
	let promise = promiseCreator();
	// similar to async JS function call. Returns a Promise instance
	let res = etsVm.call('testAwaitJsPromise', promise);
	msg('Called testAwaitJsPromise OK, result:', INFO);
	msg(res, INFO);
	if (typeof res !== 'object') {
		msg('Result is not an object', FATAL);
		process.exit(1);
	}
	if (res.constructor.name !== 'Promise') {
		msg("Expect result type 'Promise' but get '" + res.constructor.name + "'", FATAL);
		process.exit(1);
	}
	let anotherRes = doAwait(res);
	msg('Called doAwait OK, res: ', INFO);
	msg(anotherRes, INFO);
	queueMicrotask(() => {
		if (testSuccess) {
			msg('Promise must not be resolved until JS resolves the passed one', FATAL);
			process.exit(1);
		}
		// resolve the passed promise if necessary
		promiseResolver(PROMISE_RESOLVE_VALUE);
		// after Q is processed, the test should pass
		queueMicrotask(async () => {
			msg('Starting await of doAwait() result, its state is:', INFO);
			msg(anotherRes, INFO);
			await anotherRes;
			if (!testSuccess) {
				msg('Promise is not resolved or value is wrong', FATAL);
				process.exit(1);
			} else {
				msg('Test PASSED', INFO);
			}
		});
	});
}

function runAwaitPendingTest() {
	runAwaitTest('AwaitPendingTest', createPendingPromise, (v) => {
		resolver(v);
	});
}

function runAwaitResolvedTest() {
	runAwaitTest('AwaitResolvedTest', createResolvedPromise, (v) => {});
}

function runTest(test) {
	if (test === 'pending') {
		runAwaitPendingTest();
	} else if (test === 'resolved') {
		runAwaitResolvedTest();
	} else {
		msg('No such test', FATAL);
		process.exit(1);
	}
}

process.on('uncaughtException', function (err) {
	msg(err.stack, ERROR);
	throw err;
});

let args = process.argv;
if (args.length !== 3) {
	msg('Expected test name', FATAL);
	process.exit(1);
}
runTest(args[2]);
