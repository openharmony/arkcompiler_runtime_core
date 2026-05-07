/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

const object_count = 20;
const promise_count = 30;

let token = new Object();
let registry: FinalizationRegistry<String> | null = null;

let obj_array: Object[] = new Object[object_count](new Object());


function doFullGC() {
	try {
		GC.waitForFinishGC(GC.startGC(GC.Cause.FULL, GC.IN_PLACE_MODE));
	} catch (e) {
	 arktest.assertTrue(false, 'Unexpected error during GC');
	}
}

async function doFullGCAsync() {
	try {
		GC.waitForFinishGC(GC.startGC(GC.Cause.FULL, GC.IN_PLACE_MODE));
	} catch (e) {
	 arktest.assertTrue(false, 'Unexpected error during GC');
	}
}


function addToRegistry(finreg: FinalizationRegistry<String> | null, count: Int, token: Object): void {

	for (let i = 0; i < count; i++) {
		finreg!.register(new Object(), 'test', token);
	}
}

function removeFromRegistry(finreg: FinalizationRegistry<String> | null, token: Object): void {
	finreg!.unregister(token);
}

// ==============================  GetSize tests ==================

let promise_array: Array<Promise<void>> = new Array<Promise<void>>;

async function getSizeTests(finreg: FinalizationRegistry<String> | null) {

	for (let i = 0; i < promise_count; i++) {
		promise_array.push(launch() {
			addToRegistry(finreg, 3, token)
		});
	}

	for (let i = 0; i < promise_array.length; i++) {
		await promise_array[i];
	}

 arktest.assertEQ(3 * promise_count, finreg!.getSize(), 'Size of the finreg is invalid');
}

// ========================= Unregister tests ===========================================

let unreg_array: Array<Promise<void>> = new Array<Promise<void>>;

async function unregisterTests(finreg: FinalizationRegistry<String> | null) {

	finreg!.unregister(token);
 arktest.assertEQ(0, finreg!.getSize(), 'Start size of unregister is invalid');

	let token1 = new Object();
	let token2 = new Object();
	let token3 = new Object();
	let token4 = new Object();

	addToRegistry(finreg, object_count, token1);
	addToRegistry(finreg, object_count, token2);
	addToRegistry(finreg, object_count, token3);
	addToRegistry(finreg, object_count, token4);

 arktest.assertEQ(object_count * 4, finreg!.getSize(), 'Size after register of Unregister test is invalid');

	const unregister = (finreg: FinalizationRegistry<String>, s: Object) => {
		return finreg.unregister(s);
	};

	await launch() {
		unregister(finreg!, token1)
	};
	await launch() {
		unregister(finreg!, token2)
	};
	await launch() {
		unregister(finreg!, token3)
	};
	await launch() {
		unregister(finreg!, token4)
	};

 arktest.assertEQ(0, finreg!.getSize(), 'Size of unregister is invalid');

	for (let i = 0; i < obj_array.length; i++) {
		addToRegistry(finreg!, object_count, obj_array[i]);
	}

 arktest.assertEQ(obj_array.length * object_count, finreg!.getSize(), 'Register function is invalid');

	for (let i = 0; i < obj_array.length; i++) {
		unreg_array.push(launch() {
			removeFromRegistry(finreg!, obj_array[i])
		});
	}

	for (let i = 0; i < unreg_array.length; ++i) {
		await unreg_array[i];
	}
 arktest.assertEQ(0, finreg!.getSize(), 'unregister  test failed');
}

// ===============================  CtorTests ==============================

let regs_promise: Array<Promise<void>> = new Array<Promise<void>>;
let ctor_regs: Array<FinalizationRegistry<String>> = new Array<FinalizationRegistry<String>>;
let cntr = 0;
const ctor_cleanup_count = promise_count * 2;
let ctor_cleanup_resolve: (() => void) | null = null;

function onCtorCleanup(): void {
	cntr++;
	if (cntr == ctor_cleanup_count) {
		ctor_cleanup_resolve!();
	}
}


function createCoro(idx: int): FinalizationRegistry<String> {
	let finreg = new FinalizationRegistry<String>((string: String): void => { cntr++ });
	let o = new Object();
	finreg.register(o, "<<< :: >>> " + idx, o);
	return finreg;
}


async function createCoroA(idx: int, noToken: boolean): Promise<void> {
    let finreg = new FinalizationRegistry<String>((s: String): void => { onCtorCleanup(); });
    if (noToken) {
        finreg.register(new Object(), "<<< :Async1: >>> " + idx);
    } else {
		let token = new Object();
        finreg.register(new Object(), "<<< :Async2: >>> " + idx, token);
    }
	ctor_regs.push(finreg);
}

async function ctorTests() {
    cntr = 0;
    regs_promise = new Array<Promise<void>>;
    ctor_regs = new Array<FinalizationRegistry<String>>;
	let cleanupFinished = new Promise<void>((resolve) => {
		ctor_cleanup_resolve = () => { resolve(undefined); };
	});

    for (let i = 0; i < promise_count; i++) {
        regs_promise.push(createCoroA(i, true));
	    regs_promise.push(createCoroA(i, false));
	}

    for (let i = 0; i < regs_promise.length; i++) {
		await regs_promise[i];
	}

	regs_promise = new Array<Promise<void>>;
	doFullGC();
	await cleanupFinished;
	ctor_regs = new Array<FinalizationRegistry<String>>;
	ctor_cleanup_resolve = null;

}

async function runAllTests() {
    registry = new FinalizationRegistry<String>((string: String): void => { });
	await getSizeTests(registry!);
	await unregisterTests(registry!);

	await ctorTests();
	arktest.assertEQ(cntr, ctor_cleanup_count, "Counter of ctors is not valid");
}

async function main(): Promise<void> {
	await runAllTests();
}
