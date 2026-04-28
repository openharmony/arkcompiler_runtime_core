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

async function testConcurrentGet(): Promise<void> {
	const JOB_COUNT = 7;
	const SET_COUNT = 100;

	let map = new CHMap();
	for (let i = 0; i < JOB_COUNT * SET_COUNT; ++ i)
		map.set(i, i);

	let makeGetF = (start: int,  end: int) => () => {
		for (let i = start; i < end; ++ i)
			arktest.assertEQ(map.get(i), i);
	}
	let getJobs: Promise<void>[] = [];
	for (let i = 0; i < JOB_COUNT; ++ i) {
		let start = SET_COUNT * i;
		addLaunch(getJobs, makeGetF(start, start + SET_COUNT));
	}
	await awaitAll(getJobs);
}

async function testConcurrentGet2(): Promise<void> {
	const JOB_COUNT = 10;
	const SET_COUNT = 100;

	let map = new CHMap();
	for (let i = 0; i < SET_COUNT; ++ i) map.set(i, i);

	let getF = () => {
		for (let i = 0; i < SET_COUNT; ++ i)
			arktest.assertEQ(map.get(i), i);
	}
	let getJobs: Promise<void>[] = [];
	for (let i = 0; i < JOB_COUNT; ++ i) {
		addLaunch(getJobs, getF);
	}
	await awaitAll(getJobs);
}

function main(): int {
    let CHMTestSuitecase  = new arktest.ArkTestsuite("ConcurrentHashMapTest")
	CHMTestSuitecase.addAsyncTest("testConcurrentGet", async () => { await testConcurrentGet(); });
	CHMTestSuitecase.addAsyncTest("testConcurrentGet2", async () => { await testConcurrentGet2(); });

    return CHMTestSuitecase.run();
}
