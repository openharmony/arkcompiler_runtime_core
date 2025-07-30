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

function RunTasksTest(): void {
    let mainId = CoroutineExtras.getWorkerId();
    let w = new EAWorker(true);
    let p1 = w.run<int>(():int => {
        let runId = CoroutineExtras.getWorkerId();
        assertNE(runId, mainId);
        return runId;
    });
    let p2 = w.run<int>(():int => {
        let runId = CoroutineExtras.getWorkerId();
        assertNE(runId, mainId);
        return runId;
    });
    w.join();
    let runId1 = p1.Await();
    let runId2 = p2.Await();
    assertEQ(runId1, runId2);
}

function RunTasksWithJsCallTest(): void {
    let mainId = CoroutineExtras.getWorkerId();
    let w = new EAWorker(true);
    let callback = ():int => {
        let module = JSRuntime.loadModule("./eaworker_test_helpers_js");
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

    let p1 = w.run<int>(callback);
    let p2 = w.run<int>(callback);
    w.join();
    let result1 = p1.Await();
    let result2 = p2.Await();
    assertEQ(result1, 0);
    assertEQ(result2, 0);
}

function RunTasksWithJsAsyncCallTest(): void {
    let w = new EAWorker(true);
    let callback = ():int => {
        let module = JSRuntime.loadModule("./eaworker_test_helpers_js");
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

    let p = w.run<int>(callback);
    w.join();
    let result = p.Await();
    assertEQ(result, 0);
}

function CreateEAWorkerWithoutInterop(): void {
    let eaw = new EAWorker();
    let res = false;
    try {
        let callback = () => {
            let module = JSRuntime.loadModule('./eaworker_test_helpers_js');
            let val = JSRuntime.getPropertyJSValue(module, 'returnTwo');
        };
        eaw.run<void>(callback).Await();
    } catch(e: Error) {
        res = true;
        eaw.join();
    }
    assertEQ(res, true);
}
