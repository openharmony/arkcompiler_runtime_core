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

async function testConcurrentDelete(): Promise<void> {
	const JOB_COUNT = 7;
	const SET_COUNT = 100;

	let map = new CHMap();

	let deleteJobs: Promise<void>[] = [];
	let makeDeleteF = (start: int, end: int, shouldBe: boolean) => () => {
		for (let i = start; i < end; ++ i)
			arktest.assertEQ(map.delete(i), shouldBe);
	}
	for (let i = 0; i < JOB_COUNT; ++ i)
		addLaunch(deleteJobs, makeDeleteF(i * SET_COUNT, (i + 1) * SET_COUNT, false));
	await awaitAll(deleteJobs);

	deleteJobs = [];
	for (let i = 0; i < SET_COUNT * JOB_COUNT; ++ i)
		map.set(i, i);
	for (let i = 0; i < JOB_COUNT; ++ i)
		addLaunch(deleteJobs, makeDeleteF(i * SET_COUNT, (i + 1) * SET_COUNT, true));
	await awaitAll(deleteJobs);
	for (let i = 0; i < JOB_COUNT * SET_COUNT; ++ i)
		arktest.assertFalse(map.has(i));
}

async function testConcurrentDelete2(): Promise<void> {
	const JOB_COUNT = 10;
	const SET_COUNT = 100;

	let map = new CHMap();

	let deleteJobs: Promise<FixedArray<int>>[] = [];
	let deleteF: ()=>FixedArray<int> = () => {
		let arr: FixedArray<int> = new FixedArray<int>(SET_COUNT);
		for (let i = 0; i < SET_COUNT; ++ i)
			if(map.delete(i))
				arr[i] = 1;
		return arr;
	}

	for (let i = 0; i < SET_COUNT; ++ i)
		map.set(i, i);
	for (let i = 0; i < JOB_COUNT; ++ i)
		addLaunch(deleteJobs, deleteF);
	let arrs = await awaitAll(deleteJobs);
	let accum = arrs[0];
	for (let i = 1; i < JOB_COUNT; ++ i )
		sumArrInto(accum, arrs[i]);
	assertEQAll(accum, 1);
}

async function testConcurrentDeleteSetAndHas(): Promise<void> {
	const JOB_COUNT = 5;
	const SET_COUNT = 70;

	let map = new CHMap();
	let setF = () => {
		for (let i = 0; i < SET_COUNT; ++ i) {
			map.set(i, i);
		}
	}
	let deleteF = () => {
		for (let i = 0; i < SET_COUNT; ++ i) {
			map.delete(i);
		}
	}
	let hasF = () => {
		for (let i = 0; i < SET_COUNT; ++ i) {
			map.has(i);
		}
	}
	let setJobs: Promise<void>[] = [];
	let deleteJobs: Promise<void>[] = [];
	let hasJobs: Promise<void>[] = [];
	for (let i = 0; i < JOB_COUNT; ++ i) {
		addLaunch(setJobs, setF);
		addLaunch(deleteJobs, deleteF);
		addLaunch(hasJobs, hasF);
	}
	await awaitAll(setJobs);
	await awaitAll(deleteJobs);
	await awaitAll(hasJobs);
}

function main(): int {
    let CHMTestSuitecase  = new arktest.ArkTestsuite("ConcurrentHashMapTest")
	CHMTestSuitecase.addAsyncTest("testConcurrentDelete", async () => { await testConcurrentDelete(); });
	CHMTestSuitecase.addAsyncTest("testConcurrentDelete2", async () => { await testConcurrentDelete2(); });
	CHMTestSuitecase.addAsyncTest("testConcurrentDeleteSetAndHas", async () => { await testConcurrentDeleteSetAndHas(); });

    return CHMTestSuitecase.run();
}
