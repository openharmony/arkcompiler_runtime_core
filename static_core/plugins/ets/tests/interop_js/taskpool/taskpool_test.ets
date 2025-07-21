/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

function RunTasksWithJsCallTest(): void {
    let mainId = CoroutineExtras.getWorkerId();
    let callback = ():int => {
        let module = JSRuntime.loadModule("./taskpool_interop_tests_helpers_js");
        let val = JSRuntime.getPropertyJSValue(module, 'returnTwo')();
        try {
            assertEQ(val as int, 2);
        }
        catch(e: Error) {
            console.log("RunTasksWithJsCallTest failed");
            return 1;
        }
        return 0;
    };

    let p1 = taskpool.execute(callback);
    let p2 = taskpool.execute(callback);
    let result1 = await p1;
    let result2 = await p2;
    assertEQ(result1, 0);
    assertEQ(result2, 0);
}

function RunTasksWithJsAsyncCallTest(): void {
    let callback = ():int => {
        let module = JSRuntime.loadModule("./taskpool_interop_tests_helpers_js");
        let modified = false;
        let promise = JSRuntime.getPropertyJSValue(module, 'asyncWithAwait')() as Promise<Object>;
        let promise1 = promise.then((x:Object): void => {
            modified = true;
        });

        await promise1;

        try {
            let value = JSRuntime.getPropertyJSValue(JSRuntime.getGlobal(), 'value') as int;
            assertEQ(value, 7);
            assertEQ(modified, true);
        }
        catch(error) {
            console.log("Value modifications failed");
            return 1;
        }

        return 0;
    };

    let p = taskpool.execute(callback);
    let result = await p;
    assertEQ(result, 0);
    assertEQ(CoroutineExtras.isTaskpoolSupportingInterop(), true);
    CoroutineExtras.stopTaskpool();
}

// This is a pure CPU task without IO request
function additionDelay(delay: number): void {
    let start: number = new Date().getTime();
    while (new Date().getTime() - start < delay) {
        continue;
    }
}

function testCPUTaskDuration(): void {    
    let task: taskpool.Task = new taskpool.Task(additionDelay, 300.0);
    let runner: taskpool.SequenceRunner = new taskpool.SequenceRunner();

    // All initialized with 0
    arktest.assertTrue(task.ioDuration == 0);
    arktest.assertTrue(task.cpuDuration == 0);
    arktest.assertTrue(task.totalDuration == 0);

    await runner.execute(task); 

    arktest.assertTrue(task.cpuDuration > 0);
    arktest.assertTrue(task.totalDuration > 0);
    arktest.assertTrue(task.ioDuration == 0);
}

export function main(): int {
    let testSuite = new arktest.ArkTestsuite('taskpool.taskpool_test');
    testSuite.addTest('testCPUTaskDuration', testCPUTaskDuration);
    let res = testSuite.run();
    return res;
}