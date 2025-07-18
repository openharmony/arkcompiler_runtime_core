/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

type BlockingQueueI = containers.BlockingQueue<int>;
type BQConstructorI = (capacity: int)=>BlockingQueueI;
type TestFunction = (constr: BQConstructorI) => void;
type BlockingQueue = containers.LinkedBlockingQueue<int> | containers.ArrayBlockingQueue<int>;

function addLaunch<T> (arr: Array<Job<T>>, func: ()=>T): void {
	arr.push(launch <T, ()=>T>(func));
}

function awaitAll <T> (arr: Array<Job<T>>): Array<T> {
	let res: Array<T> = new Array<T> (arr.length);
	for (let i = 0; i < arr.length; ++ i)
		res[i] = arr[i].Await();
	return res;
}

function sumArrInto (to: FixedArray<int>, from: FixedArray<int>): void {
	arktest.assertEQ(to.length, from.length);
	for (let i = 0; i < to.length; ++ i) {
		to[i] += from[i];
	}
}

function subArrInto(to: FixedArray<int>, from: FixedArray<int>): void {
	arktest.assertEQ(to.length, from.length);
	for (let i = 0; i < to.length; ++ i) {
		to[i] -= from[i];
	}
}

function assertEQAll(array: FixedArray<int>, value: int): void {
	for (let i=0; i < array.length; ++ i)
		arktest.assertEQ(array[i], value)
}

function testOrderAddAndPollMany(constr: BQConstructorI) {
	let queue = constr(50);
	const EXTRA = -1;
	let adders: Job<Array<int>>[] = [];
	let pollers: Job<Array<int>>[] = [];

	const ADD_COUNT = 100;
	const JOB_COUNT = 10;

	let makeAddF = (seed: int) => () => {
		let lost = new Array<int>();
		for (let i = 0; i < ADD_COUNT; ++ i) {
			let val = i + seed * ADD_COUNT;
			if (! queue.add(val))
				lost.push(val);
		}

		while(!queue.add(EXTRA)){ Coroutine.Schedule(); }
		return lost;
	};

	let pollF = () => {
		let found = new Array<int>();
		let prev: FixedArray<int> = new int [JOB_COUNT];
		for (let i = 0; i < JOB_COUNT; ++ i) prev[i] = -1;
		while (true) {
			let val = queue.poll();
			if (val == undefined) continue;
			if (val == EXTRA) break;
			found.push(val as int);
			let key = val / ADD_COUNT;
			val = val as int % ADD_COUNT;
			arktest.assertLT(prev[key], val);
			Coroutine.Schedule();
		}
		return found;
	};

	for (let i = 0; i < JOB_COUNT; ++i) {
		addLaunch(adders, makeAddF(i));
		addLaunch(pollers, pollF);
	}

	// each must be found in poll or lost in add
	let check: FixedArray<int> = new int [JOB_COUNT * ADD_COUNT]; 	awaitAll(adders);
	let lost = awaitAll(adders);
	let found = awaitAll(pollers);
	lost.forEach(arr => arr.forEach(i => { ++check[i] }));
	found.forEach(arr => arr.forEach(i => { ++check[i] }));
	// Found = 1, lost = 1, nothing = 0, both = 2. Only 1 is good
	assertEQAll(check, 1);
}


function testAddAndPollCount(constr: BQConstructorI): void {
	let queue = constr(50);
	let adders: Job<FixedArray<int>>[] = [];
	let pollers: Job<FixedArray<int>>[] = [];

	const ADD_COUNT = 100;
	const JOB_COUNT = 10;

	const EXTRA = -1;

	let addFunction: ()=>FixedArray<int> = ()=>{
		let added: FixedArray<int> = new int[ADD_COUNT];
		for (let i = 0; i < ADD_COUNT; ++i) {
			if (queue.add(i))
				++ added[i];
		}
		while (!queue.add(EXTRA)){ Coroutine.Schedule(); }
		return added;
	};

	let pollFunction: ()=>FixedArray<int> = () => {
		let polled: FixedArray<int> = new int[ADD_COUNT];
		while (true) {
			let val = queue.poll();
			if (val == undefined)
				continue;
			if (val == EXTRA) {
				return polled;
			}
			++ polled[val];
			Coroutine.Schedule();
		}
	};
	for (let i = 0; i < JOB_COUNT; ++i) {
		addLaunch(adders, addFunction);
		addLaunch(pollers, pollFunction);
	}

	let finalCounter: FixedArray<int> = new int[ADD_COUNT];
	let addedCounter = awaitAll(adders);
	let polledCounter = awaitAll(pollers);
	for (let i = 0; i < JOB_COUNT; ++ i) {
		sumArrInto(finalCounter, addedCounter[i]);
		subArrInto(finalCounter, polledCounter[i]);
	}
	assertEQAll(finalCounter, 0); // Each added must be polled
	arktest.assertTrue(queue.isEmpty());
}

function main(): int {
	let tests: Array<[String, TestFunction]> = [
		["testOrderAddAndPollMany", testOrderAddAndPollMany],
		["testAddAndPollCount", testAddAndPollCount],
	];

	let DLTestSuitecase = new arktest.ArkTestsuite("DeadLockBlockingQueueTests")
	let LBQconstr: BQConstructorI = (capacity: int)=>new containers.LinkedBlockingQueue<int>(capacity);
	let ABQconstr: BQConstructorI = (capacity: int)=>new containers.ArrayBlockingQueue<int>(capacity);

	for (let i = 0; i < tests.length; ++ i) {
		let name = tests[i][0];
		let testF = tests[i][1];
		DLTestSuitecase.addTest("LinkedBQ" + name, ()=>testF(LBQconstr));
		DLTestSuitecase.addTest("ArrayBQ" + name, ()=>testF(ABQconstr));
	}

	return LBQTestSuitecase.run();
}

