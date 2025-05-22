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

let pressureFlag: number = 0;

let concurrentHashMapOne: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();

function main(): int {

    let concurrentHashMapTestsuite = new ArkTestsuite("concurrentHashMapPressureTest");

    concurrentHashMapTestsuite.addTest("setPressureTest_001", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        let promises = new Array<Promise<void>>(100);
        pressureFlag = 0;
        for (let i: int = 0; i < 100; i++) {
            promises[i] = launch setForPressure(pressureFlag);
            pressureFlag += 1;
        }
        for (let i: int = 0; i < 100; i++) {
            await promises[i];
        }
        assertEQ(concurrentHashMapOne.size, 100);
        for (let i: int = 0; i < 100; i++) {
            let j = `${i}`;
            assertEQ(concurrentHashMapOne.get(i), j);
        }
    });

    return concurrentHashMapTestsuite.run();
}

function setForPressure(pressureFlag: number) {
    let num = pressureFlag;
    for (let i: number = num; i < num + 1; i++) {
        let j: string = `${i}`;
        concurrentHashMapOne.set(i, j);
    }
}
