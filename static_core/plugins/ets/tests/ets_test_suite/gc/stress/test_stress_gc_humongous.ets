/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

 // This test tries to load gc with a large number of memory allocations 
 // close to the OOM situation

const NUMBER_OF_FREE_REGIONS = 2;

function cor(numObjToCreate: int, humongousSize: int): Object | null {
    let localGarbage = new ArrayAsListObject();
    try {
        for (let i = 0; i < numObjToCreate; i++) {
            localGarbage.pushBack(new byte[humongousSize] as Object);
        }
    }
    catch(e: OutOfMemoryError) {
        console.println("Catched OOM, just return");
        return null;
    }
    catch(e) {
        assert false : "Some other error: " + e.toString();
    }
    return localGarbage as Object;
};

class TestHumongousStress {
    private static corArray = new ArrayAsListObject();
    private static GARBAGE = new ArrayAsListObject();

    regionSize: int;
    humongousSize: int;
    threadsCount: int;

    constructor (regionSizeInMb: int, threadsCount: int, humongousSizeMultiplier: double) {
        this.regionSize = regionSizeInMb * 1024 * 1024;
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
            let numOfObj = TestHumongousStress.GARBAGE.size();
            for(let i = 0; i < numOfObj; i++) {
                TestHumongousStress.GARBAGE.popFront();
            }
        }
        catch(e) {
            assert false : "Some other error: " + e.toString();
        }
        return maxNumberOfObjects;
    }

    public run(): void {
        let maxObjects = this.getHeapSizeInObjects() - NUMBER_OF_FREE_REGIONS;
        let objPerCoroutine = (maxObjects / this.threadsCount) as int;
        let lastCoroutuneNumObjects = maxObjects - objPerCoroutine * (this.threadsCount - 1);
        assert this.threadsCount > 0 && objPerCoroutine > 0;

        let corArrayTmp = new Promise<Object>[this.threadsCount - 1];
        for(let i = 0; i < this.threadsCount - 1; i++) {
            corArrayTmp[i] = launch cor(objPerCoroutine, this.humongousSize);
        }
        let corArrayLast = launch cor(lastCoroutuneNumObjects, this.humongousSize);

        Coroutine.Schedule();

        for(let i = 0; i < this.threadsCount - 1; i++) {
            let tmp = (await corArrayTmp[i]) as ArrayAsListObject;
            if(tmp == null) {
                console.println("Null is returned. Not enough memory.Finish.");
                return;
            }
            for(let j = 0; j < objPerCoroutine; j++) {
                TestHumongousStress.GARBAGE.pushBack(tmp.popFront());
            }
        }
        let tmp = (await corArrayLast) as ArrayAsListObject;
        if(tmp == null) {
                console.println("Null is returned. Not enough memory.Finish.");
                return;
            }
        for(let j = 0; j < objPerCoroutine; j++) {
            TestHumongousStress.GARBAGE.pushBack(tmp.popFront());
        }
    }

    public clear(): void {
        let numOfObj = TestHumongousStress.GARBAGE.size();
        for(let i = 0; i < numOfObj; i++) {
            TestHumongousStress.GARBAGE.popFront();
        }
    }
}

function main(): void {
    let tests = new ArrayAsListObject();
    tests.pushBack(new TestHumongousStress(4, 3, 1.1));
    tests.pushBack(new TestHumongousStress(16, 5, 2.1));
    tests.pushBack(new TestHumongousStress(32, 4, 0.6));

    for (let i = 0; i < tests.size(); i++) {
        let tst = tests.popFront() as TestHumongousStress;
        tst.run();
        tst.clear();
    }
}