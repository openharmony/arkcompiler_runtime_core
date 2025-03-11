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

let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"];

let concurrentHashMapOne: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();

function main(): int {
    let map: Map<number, string> = new Map<number, string>();
    for (let i: int = 0; i < 10 ; i++) {
        map.set(arrKeys[i], arrValues[i]);
    }

    let concurrentHashMapTestsuite = new ArkTestsuite("concurrentHashMapMoreTypeTest");
    // K: number, V: boolean
    concurrentHashMapTestsuite.addTest("Number&Boolean_001", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, boolean> = new ConcurrentHashMap<number, boolean>();
        assertEQ(concurrentHashMap.size, 0);
        for (let i = 0; i < 10; i++) {
            let tf = (i % 2) == 0;
            concurrentHashMap.set(i, tf);
        }
        assertEQ(concurrentHashMap.size, 10);
        for (let i = 0; i < 10; i++) {
            assertEQ(concurrentHashMap.get(i), true);
            i++;
        }
        for (let i = 1; i < 10; i++) {
            assertEQ(concurrentHashMap.get(i), false);
            i++;
        }
    });
    concurrentHashMapTestsuite.addTest("Number&Boolean_002", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, boolean> = new ConcurrentHashMap<number, boolean>();
        assertEQ(concurrentHashMap.size, 0);
        for (let i = 0; i < 10; i++) {
            let tf = (i % 2) == 0;
            concurrentHashMap.set(i, tf);
        }
        assertEQ(concurrentHashMap.size, 10);
        for (let i = 0; i < 10; i++) {
            let val = concurrentHashMap.get(i) as boolean;
            if (val) {
                concurrentHashMap.replace(i, false);
            } else {
                concurrentHashMap.replace(i, true);
            }
        }
        assertEQ(concurrentHashMap.size, 10);
        for (let i = 0; i < 10; i++) {
            assertEQ(concurrentHashMap.get(i), false);
            i++;
        }
        for (let i = 1; i < 10; i++) {
            assertEQ(concurrentHashMap.get(i), true);
            i++;
        }
    });
    concurrentHashMapTestsuite.addTest("Number&Boolean_003", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, boolean> = new ConcurrentHashMap<number, boolean>();
        assertEQ(concurrentHashMap.size, 0);
        for (let i = 0; i < 10; i++) {
            let tf = (i % 2) == 0;
            concurrentHashMap.set(i, tf);
        }
        assertEQ(concurrentHashMap.size, 10);
        for (let i = 0; i < 10; i++) {
            assertTrue(concurrentHashMap.delete(i));
            assertEQ(concurrentHashMap.size, 9 - i);
        }
    });
    concurrentHashMapTestsuite.addTest("Number&Boolean_004", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, boolean> = new ConcurrentHashMap<number, boolean>();
        assertEQ(concurrentHashMap.size, 0);
        for (let i = 0; i < 10; i++) {
            let tf = (i % 2) == 0;
            concurrentHashMap.set(i, tf);
        }
        assertEQ(concurrentHashMap.size, 10);
        for (let i = 0; i < 10; i++) {
            let res = concurrentHashMap.remove(i) as boolean;
            assertEQ(res, (i % 2) == 0);
        }
        assertEQ(concurrentHashMap.size, 0);
    });
    concurrentHashMapTestsuite.addTest("Number&Boolean_005", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, boolean> = new ConcurrentHashMap<number, boolean>();
        assertEQ(concurrentHashMap.size, 0);
        for (let i = 0; i < 10; i++) {
            let tf = (i % 2) == 0;
            concurrentHashMap.set(i, tf);
        }
        assertEQ(concurrentHashMap.size, 10);
        let iter_NB = concurrentHashMap.entries();
        for (let i = 0; i < 10; i++) {
            if ((i % 2) == 0) {
                assertTrue(iter_NB.next().value![1]);
            } else {
                assertFalse(iter_NB.next().value![1]);
            }
        }
    });
    concurrentHashMapTestsuite.addTest("Number&Boolean_006", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, boolean> = new ConcurrentHashMap<number, boolean>();
        assertEQ(concurrentHashMap.size, 0);
        for (let i = 0; i < 10; i++) {
            let tf = (i % 2) == 0;
            concurrentHashMap.set(i, tf);
        }
        concurrentHashMap.forEach((val: boolean, key: number, map: ConcurrentHashMap<number, boolean>) =>
        {
            let str = map.toString();
            assertEQ(str, "0,true,1,false,2,true,3,false,4,true,5,false,6,true,7,false,8,true,9,false");
            if (key % 2 == 0) {
                assertTrue(val);
            } else {
                assertFalse(val);
            }
        });
    });
    // K: number, V: Object
    concurrentHashMapTestsuite.addTest("Number&Object_001", () =>
    {
        let hasMapNO: ConcurrentHashMap<number, Array<string>> = new ConcurrentHashMap<number, Array<string>>();
        for (let i = 0; i < 5; i++) {
            let array: Array<string> = new Array<string>(5);
            for (let j = 0; j < i + 1; j++) {
                array[j] = arrValues[j];
            }
            hasMapNO.set(i, array);
            let arrGet = hasMapNO.get(i) as Array<string>;
            assertEQ(array, arrGet);
        }
    });
    concurrentHashMapTestsuite.addTest("Number&Object_002", () =>
    {
        let hasMapNO: ConcurrentHashMap<number, Array<string>> = new ConcurrentHashMap<number, Array<string>>();
        for (let i = 0; i < 5; i++) {
            let array: Array<string> = new Array<string>(5);
            for (let j = 0; j < i + 1; j++) {
                array[j] = arrValues[j];
            }
            hasMapNO.set(i, array);
        }
        for (let i = 0; i < 5; i++) {
            let array: Array<string> = new Array<string>(1);
            array[0] = "six";
            hasMapNO.replace(i, array);
            let arrGet = hasMapNO.get(i) as Array<string>;
            assertEQ(array, arrGet);
        }
    });
    concurrentHashMapTestsuite.addTest("Number&Object_003", () =>
    {
        let hasMapNO: ConcurrentHashMap<number, Array<string>> = new ConcurrentHashMap<number, Array<string>>();
        for (let i = 0; i < 5; i++) {
            let array: Array<string> = new Array<string>(5);
            for (let j = 0; j < i + 1; j++) {
                array[j] = arrValues[j];
            }
            hasMapNO.set(i, array);
        }
        for (let i = 0; i < 5; i++) {
            assertTrue(hasMapNO.delete(i));
            assertEQ(hasMapNO.size, 4 - i);
        }
    });
    concurrentHashMapTestsuite.addTest("Number&Object_004", () =>
    {
        let hasMapNO: ConcurrentHashMap<number, Array<string>> = new ConcurrentHashMap<number, Array<string>>();
        for (let i = 0; i < 5; i++) {
            let array: Array<string> = new Array<string>(5);
            for (let j = 0; j < i + 1; j++) {
                array[j] = arrValues[j];
            }
            hasMapNO.set(i, array);
        }
        for (let i = 0; i < 5; i++) {
            let arrRes = hasMapNO.remove(i) as Array<string>;
            for (let j = 0; j < i + 1; j++) {
                assertEQ(arrValues[j], arrRes[j]);
            }
            assertEQ(hasMapNO.size, 4 - i);
        }
    });
    concurrentHashMapTestsuite.addTest("Number&Object_005", () =>
    {
        let hasMapNO: ConcurrentHashMap<number, Array<string>> = new ConcurrentHashMap<number, Array<string>>();
        for (let i = 0; i < 5; i++) {
            let array: Array<string> = new Array<string>(5);
            for (let j = 0; j < i + 1; j++) {
                array[j] = arrValues[j];
            }
            hasMapNO.set(i, array);
        }
        let iter_NO = hasMapNO.entries();
        for (let i = 0; i < 5; i++) {
            let array: Array<string> = new Array<string>(5);
            for (let j = 0; j < i + 1; j++) {
                array[j] = arrValues[j];
            }
            let arrIter = iter_NO.next().value![1];
            for (let j = 0; j < i + 1; j++) {
                assertEQ(array[j], arrIter[j]);
            }
        }
        assertEQ(iter_NO.next().value, undefined);
    });
    concurrentHashMapTestsuite.addTest("Number&Object_006", () =>
    {
        let hasMapNO: ConcurrentHashMap<number, Array<string>> = new ConcurrentHashMap<number, Array<string>>();
        for (let i = 0; i < 5; i++) {
            let array: Array<string> = new Array<string>(5);
            for (let j = 0; j < i + 1; j++) {
                array[j] = arrValues[j];
            }
            hasMapNO.set(i, array);
        }
        hasMapNO.forEach((val: Array<string>, key: number, map: ConcurrentHashMap<number, Array<string>>) =>
        {
            assertEQ(map.get(key), val);
        });
    });

    concurrentHashMapTestsuite.addTest("entriesOfNumber", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, number> = new ConcurrentHashMap<number, number>();
        for (let i: number = 0; i < 10 ; i++) {
            concurrentHashMap.set(i, i);
        }
        let iterEntries = concurrentHashMap.entries();
        let str = "";
        for (let i: int = 0; i < 10; i++) {
            str += iterEntries.next().value!.toString();
            if (i < 9){
                str += ",";
            }
        }
        assertEQ(iterEntries.next().value, undefined);
    });
    concurrentHashMapTestsuite.addTest("entriesOfNumber", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<string, string> = new ConcurrentHashMap<string, string>();
        for (let i: int = 0; i < 10 ; i++) {
            concurrentHashMap.set(arrValues[i], arrValues[i]);
        }
        let iterEntries = concurrentHashMap.entries();
        let str = "";
        for (let i: int = 0; i < 10; i++) {
            str += iterEntries.next().value!.toString();
            if (i < 9){
                str += ",";
            }
        }
        assertEQ(iterEntries.next().value, undefined);
    });


    return concurrentHashMapTestsuite.run();
}
