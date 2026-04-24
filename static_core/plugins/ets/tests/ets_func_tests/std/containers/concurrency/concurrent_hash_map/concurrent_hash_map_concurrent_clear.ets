/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

type CHMap = containers.ConcurrentHashMap<int, int>;

function addLaunch<T> (arr: Array<Promise<T>>, func: ()=>T): void {
	arr.push(launch(func));
}

async function awaitAll <T> (arr: Array<Promise<T>>): Promise<Array<T>> {
	let res: T[] = [];
	for (let i = 0; i < arr.length; ++ i)
		res.push((await arr[i]) as T);
	return res;
}

function sumArrInto (to: FixedArray<int>, from: FixedArray<int>): void {
	arktest.assertEQ(to.length, from.length);
	for (let i = 0; i < to.length; ++ i) {
		to[i] += from[i];
	}
}

function assertEQAll(array: FixedArray<int>, value: int): void {
	for (let i=0; i < array.length; ++ i)
		arktest.assertEQ(array[i], value)
}

async function testConcurrentClearAndSet(): Promise<void> {
	let map = new CHMap();
	let JOB_COUNT = 7;
	let SET_COUNT = 100;

	let setF = ()=>{
		for (let i = 0; i < SET_COUNT; ++ i)
			map.set(i, i);
	}

	let setJobs: Promise<void>[] = [];
	let clearJobs: Promise<void>[] = [];

	for (let i = 0; i < JOB_COUNT; ++ i) {
		addLaunch(setJobs, setF);
		addLaunch(clearJobs, ()=>map.clear());
	}
	await awaitAll(setJobs);
	await awaitAll(clearJobs);
}


async function testConcurrentClearAndGet(): Promise<void> {
	let map = new CHMap();
	let JOB_COUNT = 10;
	let SET_COUNT = 100;

	for (let i = 0; i < SET_COUNT; ++ i)
		map.set(i, i);

	let getF = ()=>{
		for (let i = 0; i < SET_COUNT; ++ i) {
			let val = map.get(i)
			if (val != undefined)
				arktest.assertEQ(val, i);
		}
	}

	let getJobs: Promise<void>[] = [];

	for (let i = 0; i < JOB_COUNT; ++ i) {
		addLaunch(getJobs, getF);
	}
	await launch(map.clear);
	await awaitAll(getJobs);
	arktest.assertTrue(map.isEmpty());
}

async function testConcurrentClearAndRemove(): Promise<void> {
	let map = new CHMap();
	let JOB_COUNT = 10;
	let SET_COUNT = 100;

	for (let i = 0; i < SET_COUNT; ++ i)
		map.set(i, i);

	let rmF = ()=>{
		for (let i = 0; i < SET_COUNT; ++ i) {
			let val = map.remove(i)
			if (val != undefined)
				arktest.assertEQ(val, i);
		}
	}

	let rmJobs: Promise<void>[] = [];

	for (let i = 0; i < JOB_COUNT; ++ i) {
		addLaunch(rmJobs, rmF);
	}
	await launch(map.clear);
	await awaitAll(rmJobs);
	arktest.assertTrue(map.isEmpty());
}

async function testConcurrentSizeAndSR1(): Promise<void> {
	let map = new CHMap();

	const JOB_COUNT = 10;
	const SET_COUNT = 100;

	let srF = () => {
		for (let i = 0; i < SET_COUNT; ++ i) {
			map.set(1, 1);
			map.remove(1);
		}
	}
	let running = true;
	let sizeF = () => {
		while (running) {
			arktest.assertLT(map.size, 2);
			Coroutine.Schedule();
		}
	}
	let sizeJob = launch(sizeF);
	let srJobs: Promise<void>[] = [];
	for (let i = 0; i < JOB_COUNT; ++ i)
		addLaunch(srJobs, srF);
	await awaitAll(srJobs);
	running = false;
	await sizeJob;
}

async function testConcurrentSizeAndSRI(): Promise<void> {
	let map = new CHMap();

	const JOB_COUNT = 10;
	const SET_COUNT = 100;

	let srF = () => {
		for (let i = 0; i < SET_COUNT; ++ i) {
			map.set(i, i);
			map.remove(i);
		}
	}
	let running = true;
	let sizeF = () => {
		while (running) {
			arktest.assertLT(map.size, SET_COUNT + 1);
			Coroutine.Schedule();
		}
	}
	let sizeJob = launch(sizeF);
	let srJobs: Promise<void>[] = [];
	for (let i = 0; i < JOB_COUNT; ++ i)
		addLaunch(srJobs, srF);
	await awaitAll(srJobs);
	running = false;
	await sizeJob;
}

function main(): int {
    let CHMTestSuitecase  = new arktest.ArkTestsuite("ConcurrentHashMapTest")
	CHMTestSuitecase.addAsyncTest("testConcurrentClearAndSet", async () => { await testConcurrentClearAndSet(); });
	CHMTestSuitecase.addAsyncTest("testConcurrentClearAndGet", async () => { await testConcurrentClearAndGet(); });
	CHMTestSuitecase.addAsyncTest("testConcurrentClearAndRemove", async () => { await testConcurrentClearAndRemove(); });

    return CHMTestSuitecase.run();
}
