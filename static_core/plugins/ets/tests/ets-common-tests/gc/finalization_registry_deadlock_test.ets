/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

let resolve: ((value: String) => void) | null = null;
     
let p = new Promise<String>((_resolve: (value: String) => void): void => {
        resolve = _resolve;
});

let finreg1: FinalizationRegistry<int> | undefined;
let finreg2: FinalizationRegistry<int> | undefined;

function runGC(): void {
    try {
        // trigger FinalizationRegistry cleanup
        let gcId = GC.startGC(GC.Cause.YOUNG);
        GC.waitForFinishGC(gcId);
    } catch (e) {
        throw new Error('Unexpected error during GC');
    }
}

class FinRegTestSuite extends arktest.ArkTestsuite {
    constructor() {
        super('FinalizationRegistrySuite');
    }

    public before() {
        finreg1 = undefined;
        finreg2 = undefined;
        GC.waitForFinishGC(GC.startGC(GC.Cause.FULL));
    }
}

function deadlockTest() {
    finreg1 = new FinalizationRegistry<int>(async (tag: int) => {
        if (tag == 1) {
            await p;
        } else if (tag == 2) {
            resolve!('Panda');
        }
    });
    // Regarding the implementation, after GC the callback will be executed 
    // first for register with number 1 and then for number 2
    finreg1!.register({}, 2);
    finreg1!.register({}, 1);
    runGC();
    Coroutine.Schedule();
    runGC();
    Coroutine.Schedule();
}

function twoFinRegsDeadlockTest() {
    finreg1 = new FinalizationRegistry<int>(async (tag: int) => {
        finreg2!.register({}, 2);
        await p;
    });
    finreg2 = new FinalizationRegistry<int>((tag: int) => {
        resolve!('Panda');
    });
    // Regarding the implementation, after GC the callback will be executed 
    // first for register with number 1 and then for number 2
    finreg1!.register({}, 1);
    runGC();
    Coroutine.Schedule();
    runGC();
    Coroutine.Schedule();
}

function main() {
    let testsuite = new FinRegTestSuite();
    testsuite.addTest('deadlockTest', deadlockTest);
    testsuite.addTest('twoFinRegsDeadlockTest', twoFinRegsDeadlockTest);
    return testsuite.run();
}
