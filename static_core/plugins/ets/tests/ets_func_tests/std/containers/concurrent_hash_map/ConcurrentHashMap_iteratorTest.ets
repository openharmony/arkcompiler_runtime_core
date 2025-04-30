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

let flag: int = 0;

let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"];
let strAll: string = "0,zero,1,one,2,two,3,three,4,four,5,five,6,six,7,seven,8,eight,9,nine";

let concurrentHashMapOne: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();

function main(): int {
    let map: Map<number, string> = new Map<number, string>();
    for (let i: int = 0; i < 10 ; i++) {
        map.set(arrKeys[i], arrValues[i]);
    }
    let concurrentHashMapIteratorTestsuite = new ArkTestsuite("concurrentHashMapIteratorTest");

    concurrentHashMapIteratorTestsuite.addTest("$_iteratorTest_001", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();
        let iter_$: IterableIterator<[Number, string]> = concurrentHashMap.$_iterator();
        assertEQ(iter_$.next().value, undefined);
    });
    concurrentHashMapIteratorTestsuite.addTest("$_iteratorTest_002", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>(map);
        let iter_$: IterableIterator<[Number, string]> = concurrentHashMap.$_iterator();
        let str = "";
        for (let i: int = 0; i < 10; i++) {
            str += iter_$.next().value!.toString();
            if (i < 9){
                str += ",";
            }
        }
        assertEQ(str, strAll);
        assertEQ(iter_$.next().value, undefined);
    });
    concurrentHashMapIteratorTestsuite.addTest("$_iteratorTest_003", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();
        let iter_$: IterableIterator<[Number, string]> = concurrentHashMap.$_iterator();
        assertEQ(iter_$.next().value, undefined);
        for (let i: int = 0; i < 10; i++) {
            concurrentHashMap.set(arrKeys[i], arrValues[i]);
        }
        iter_$ = concurrentHashMap.$_iterator();
        let str = "";
        for (let i: int = 0; i < 10; i++) {
            str += iter_$.next().value!.toString();
            if (i < 9){
                str += ",";
            }
        }
        assertEQ(str, strAll);
        assertEQ(iter_$.next().value, undefined);
    });
    concurrentHashMapIteratorTestsuite.addTest("$_iteratorTest_004", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<string, number> = new ConcurrentHashMap<string, number>();
        for (let i = 0; i < 10; i++) {
            concurrentHashMap.set(arrValues[i], arrKeys[i]);
        }
        let iter = concurrentHashMap.$_iterator();
        for (let i = 0; i < 10; i++) {
            let iterNext = iter.next();
            let k = iterNext.value![0];
            let v = iterNext.value![1];
            assertEQ(k, arrValues[i]);
            assertEQ(v, arrKeys[i]);
        }
    });
    concurrentHashMapIteratorTestsuite.addTest("$_iteratorAndTest_001", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        let p1 = launch setAll();
        let p2 = launch $_iteratorAll();
        await p1;
        await p2;
        assertEQ(concurrentHashMapOne.size, 10);
    });
    concurrentHashMapIteratorTestsuite.addTest("$_iteratorAndTest_002", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch deleteAll();
        let p2 = launch $_iteratorAll();
        await p1;
        await p2;
        assertTrue(concurrentHashMapOne.isEmpty());
    });
    concurrentHashMapIteratorTestsuite.addTest("$_iteratorAndTest_003", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch clearAll();
        let p2 = launch $_iteratorAll();
        await p1;
        await p2;
        assertTrue(concurrentHashMapOne.isEmpty());
    });

    return concurrentHashMapIteratorTestsuite.run();
}

function setAll() {
    for (let i: int = 0; i < 10; i++) {
        concurrentHashMapOne.set(arrKeys[i], arrValues[i]);
    }
}

function deleteAll() {
    for (let i: int = 0; i < 10; i++) {
        if (concurrentHashMapOne.delete(i)) {
            flag++;
        }
    }
}

function clearAll() {
    concurrentHashMapOne.clear();
}

function $_iteratorAll() {
    let iterator$ = concurrentHashMapOne.$_iterator();
    let i = 0;
    while (iterator$.next().value != undefined) {
        i++;
    }
}
