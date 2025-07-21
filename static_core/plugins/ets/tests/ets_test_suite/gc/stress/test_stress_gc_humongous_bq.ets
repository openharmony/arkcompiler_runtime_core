/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

class TestHumongousStress {
    private static GARBAGE = new LinkedBlockingQueue<Object>();

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
                TestHumongousStress.GARBAGE.push(new byte[this.humongousSize] as Object);
                maxNumberOfObjects++;
            }
        }
        catch(e: OutOfMemoryError) {
            TestHumongousStress.GARBAGE = new LinkedBlockingQueue<Object>();
            try {
                GC.waitForFinishGC(GC.startGC(GC.Cause.FULL));
            }
            catch (e) {
                assertTrue( false, "Unexpected exception during GC");
            }
        }
        catch(e) {
            assertTrue( false, "Some other error: " + e.toString());
        }
        return maxNumberOfObjects;
    }

    private static cor(numObjToCreate: int, humongousSize: int): Object|null {
        try {
            for (let i = 0; i < numObjToCreate; i++) {
                TestHumongousStress.GARBAGE.push(new byte[humongousSize] as Object);
            }
        }
        catch(e: OutOfMemoryError) {
            // NOTE: return null and localGarbage will be collected by GC
            // but as alternative it is possible to return localGarbage with
            // already allocated objects before OOM occurs
            return null;
        }
        catch(e) {
            assertTrue( false, "Some other error: " + e.toString());
        }
        return new Object();
    };

    public run(): void {
        let maxObjects = this.getHeapSizeInObjects() - NUMBER_OF_FREE_REGIONS;
        let objPerCoroutine = (maxObjects / this.threadsCount) as int;
        let lastCoroutuneNumObjects = maxObjects - objPerCoroutine * (this.threadsCount - 1);
        assertTrue( this.threadsCount > 0 && objPerCoroutine > 0);

        let corArrayTmp = new Array<Promise<Object|null>>();
        for(let i = 0; i < this.threadsCount - 1; i++) {
            corArrayTmp.push(launch<Object|null, (numObjToCreate: int, humongousSize: int) => Object|null>(TestHumongousStress.cor, objPerCoroutine, this.humongousSize));
        }
        let corArrayLast = launch<Object|null, (numObjToCreate: int, humongousSize: int) => Object|null>(TestHumongousStress.cor, lastCoroutuneNumObjects, this.humongousSize);

        Coroutine.Schedule();

        for(let i = 0; i < this.threadsCount - 1; i++) {
            let tmp = await corArrayTmp[i];
            if(tmp == null) {
                // Null is returned. Not enough memory. Finish current test.
                continue;
            }
        }
        let tmp = await corArrayLast;
        if(tmp == null) {
                // Null is returned. Not enough memory. Finish current test.
                return;
            }
    }

    public clear(): void {
        TestHumongousStress.GARBAGE = new LinkedBlockingQueue<Object>();
        try {
            GC.waitForFinishGC(GC.startGC(GC.Cause.FULL));
        }
        catch (e) {
            assertTrue( false, "Unexpected exception during GC");
        }
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