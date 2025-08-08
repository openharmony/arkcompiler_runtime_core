/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

import { launch } from "std/concurrency"
import { Job } from "std/core"

 // This test tries to load gc with a large number of memory allocations 
 // close to the OOM situation

const NUMBER_OF_FREE_REGIONS = 2;

class ArrayAsListObject {
    private data: FixedArray<Object> = [];
    private curSize: int;

    private init(capacity: int, val: Object): void {
        this.data = new Object[capacity];
        for (let i = 0; i < this.data.length; ++i) {
            this.data[i] = val;
        }
        this.curSize = capacity;
    }

    /**
    * Constructs new ArrayAsList with respect to capacity and initial value
    *
    * @param capacity
    *
    * @param val
    */
    constructor(capacity: int, val: Object) {
        this.init(capacity, val);
    }

    /**
    * Constructs new empty ArrayAsList
    */
    constructor() {
        this.init(0, new Object());
    }

    /**
    * Constructs new ArrayAsList with required capacity
    *
    * @param capacity
    */
    constructor(capacity: int) {
        this.init(capacity, new Object());
    }

    /**
    * Increases capacity if passed argument is greater than current capacity
    *
    * @param capacity
    */
    public reserve(capacity: int): void {
        if (this.data.length < capacity) {
            let newData : FixedArray<Object> = new Object[capacity];
            for (let i = 0; i < this.curSize; ++i) {
            newData[i] = this.data[i];
            }
            this.data = newData;
        }
    }

    private static getNewCapacity(currentCapacity: int): int {
        // TODO(ivan-tyulyandin): select proper capacity increasing strategy
        const fastGrowThreshold = 8192;
        const multiplier = 2;
        if (currentCapacity < fastGrowThreshold) {
            // Adding 4 to jump over 0
            return (currentCapacity + 4) * multiplier * 2;
        } else {
            return currentCapacity * multiplier;
        }
    }

    /**
    * Pops a value from the begin of the List
    *
    * @returns popped value
    */
    public popFront(): Object {
        if (this.curSize === 0) {
            throw new AssertionError("No data to popFront from ArrayAsList!")
        }
        let res: Object = this.data[0];
        for (let i = 1; i < this.curSize; ++i) {
            this.data[i-1] = this.data[i];
        }
        --this.curSize;
        return res;
    }

    /**
    * Pushes a value to the end of the List
    *
    * @param e value to push
    */
    public pushBack(e: Object): void {
        if (this.curSize == this.data.length) {
            let newData : FixedArray<Object> = new Object[ArrayAsListObject.getNewCapacity(this.data.length)];
            for (let i = 0; i < this.curSize; ++i) {
                newData[i] = this.data[i];
            }
            this.data = newData;
        }
        this.data[this.curSize] = e;
        ++this.curSize;
    }

    /**
    * Returns number of elements in the List
    */
    public size(): int {
        return this.curSize;
    }

}

function cor(numObjToCreate: int, humongousSize: int): Object|null {
    let localGarbage = new ArrayAsListObject();
    try {
        for (let i = 0; i < numObjToCreate; i++) {
            localGarbage.pushBack(new byte[humongousSize] as Object);
        }
    }
    catch(e: OutOfMemoryError) {
        // NOTE: return null and localGarbage will be collected by GC
        // but as alternative it is possible to return localGarbage with
        // already allocated objects before OOM occurs
        return null;
    }
    catch(e) {
        arktest.assertTrue( false, "Some other error: " + e.toString());
    }
    return localGarbage as Object;
};

class TestHumongousStress {
    private static GARBAGE = new ArrayAsListObject();

    regionSize: int;
    humongousSize: int;
    threadsCount: int;

    constructor (regionSizeInMb: double, threadsCount: int, humongousSizeMultiplier: double) {
        this.regionSize = (regionSizeInMb * 1024 * 1024) as int;
        this.humongousSize = (this.regionSize * humongousSizeMultiplier) as int;
        this.threadsCount = threadsCount;
    }

    private getHeapSizeInObjects(): int {
        let maxNumberOfObjects = 0;
        try {
            while(true) {
                TestHumongousStress.GARBAGE.pushBack(new byte[this.humongousSize] as Object);
                maxNumberOfObjects++;
            }
        }
        catch(e: OutOfMemoryError) {
            TestHumongousStress.GARBAGE = new ArrayAsListObject();
        }
        catch(e) {
            arktest.assertTrue( false, "Some other error: " + e.toString());
        }
        return maxNumberOfObjects;
    }

    public run(): void {
        let maxObjects = this.getHeapSizeInObjects() - NUMBER_OF_FREE_REGIONS;
        let objPerCoroutine = (maxObjects / this.threadsCount) as int;
        let lastCoroutuneNumObjects = maxObjects - objPerCoroutine * (this.threadsCount - 1);
        arktest.assertTrue(this.threadsCount > 0 && objPerCoroutine > 0);

        let corArrayTmp = new Array<Job<Object|null>>();
        for(let i = 0; i < this.threadsCount - 1; i++) {
            corArrayTmp.push(launch<Object|null, (numObjToCreate: int, humongousSize: int) => Object|null>(cor, objPerCoroutine, this.humongousSize));
        }
        let corArrayLast = launch<Object|null, (numObjToCreate: int, humongousSize: int) => Object|null>(cor, lastCoroutuneNumObjects, this.humongousSize);

        Coroutine.Schedule();

        for(let i = 0; i < this.threadsCount - 1; i++) {
            let tmp = await corArrayTmp[i];
            if(tmp == null) {
                // Null is returned. Not enough memory. Finish current test.
                continue;
            }
            let numOfReturnedObjects = (tmp as ArrayAsListObject).size();
            for(let j = 0; j < numOfReturnedObjects; j++) {
                try {
                    TestHumongousStress.GARBAGE.pushBack((tmp as ArrayAsListObject).popFront());
                }
                catch(e: OutOfMemoryError) {
                    continue;
                }
                catch(e) {
                    arktest.assertTrue( false, "Some other error: " + e.toString());
                }
            }
        }
        let tmp = await corArrayLast;
        if(tmp == null) {
                // Null is returned. Not enough memory. Finish current test.
                return;
            }
        let numOfReturnedObjects = (tmp as ArrayAsListObject).size();
        for(let j = 0; j < numOfReturnedObjects; j++) {
            try {
                TestHumongousStress.GARBAGE.pushBack((tmp as ArrayAsListObject).popFront());
            }
            catch(e: OutOfMemoryError) {
                continue;
            }
            catch(e) {
                arktest.assertTrue( false, "Some other error: " + e.toString());
            }
        }
    }

    public clear(): void {
        TestHumongousStress.GARBAGE = new ArrayAsListObject();
    }
}

function main(): void {
    let tests = new ArrayAsListObject();

    tests.pushBack(new TestHumongousStress(4, 3, 1.1));
    tests.pushBack(new TestHumongousStress(16, 5, 2.1));
    tests.pushBack(new TestHumongousStress(32, 4, 0.6));

    tests.pushBack(new TestHumongousStress(4, 12, 1.1));
    tests.pushBack(new TestHumongousStress(16, 7, 2.1));
    tests.pushBack(new TestHumongousStress(32, 18, 0.6));

    tests.pushBack(new TestHumongousStress(16, 3, 1.1));
    tests.pushBack(new TestHumongousStress(48, 2, 2.1));

    tests.pushBack(new TestHumongousStress(4, 3, 1.1));
    tests.pushBack(new TestHumongousStress(16, 3, 2.1));
    tests.pushBack(new TestHumongousStress(32, 3, 0.6));

    tests.pushBack(new TestHumongousStress(0.2, 3, 1.1));
    tests.pushBack(new TestHumongousStress(0.5, 5, 2.1));
    tests.pushBack(new TestHumongousStress(0.7, 4, 0.6));

    tests.pushBack(new TestHumongousStress(1, 7, 0.6));

    tests.pushBack(new TestHumongousStress(0.01, 19, 1.1));
    tests.pushBack(new TestHumongousStress(0.001, 39, 1.1));

    while(tests.size() > 0) {
        let tst = tests.popFront() as TestHumongousStress;
        tst.run();
        tst.clear();
    }
}