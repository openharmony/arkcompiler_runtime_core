/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

async function RunTasksTest(): Promise<void> {
    let mainId = CoroutineExtras.getWorkerId();
    let w = new EAWorker(true);
    w.start();
    let p1 = w.run<int>(():int => {
        let runId = CoroutineExtras.getWorkerId();
        arktest.assertNE(runId, mainId);
        return runId;
    });
    let p2 = w.run<int>(():int => {
        let runId = CoroutineExtras.getWorkerId();
        arktest.assertNE(runId, mainId);
        return runId;
    });
    let runId1 = await p1;
    let runId2 = await p2;
    w.quit();
    arktest.assertEQ(runId1, runId2);
}

async function RunTasksWithJsCallTest(): Promise<void> {
    let mainId = CoroutineExtras.getWorkerId();
    let w = new EAWorker(true);
    w.start();
    let callback = ():int => {
        let module = JSRuntime.loadModule("./eaworker_test_helpers_js");
        let method = JSRuntime.getPropertyJSValue(module, 'returnTwo');
        let val = JSRuntime.invoke(module, method);
        try {
            arktest.assertEQ(val, 2);
        }
        catch(e) {
            console.log("RunTasksWithJsCallTest failed");
            return 1;
        }
        return 0;
    };

    let p1 = w.run<int>(callback);
    let p2 = w.run<int>(callback);
    let result1 = await p1;
    let result2 = await p2;
    w.quit();
    arktest.assertEQ(result1, 0);
    arktest.assertEQ(result2, 0);
}

async function RunTasksWithJsAsyncCallTest(): Promise<void> {
    let w = new EAWorker(true);
    w.start();
    let callback = async (): Promise<int> => {
        let module = JSRuntime.loadModule("./eaworker_test_helpers_js");
        let modified = false;
        let method = JSRuntime.getPropertyJSValue(module, 'asyncWithAwait');
        let promise = (ESValue.wrap(JSRuntime.invoke(module, method))).toPromise();
        let promise1 = promise.then((x:Object): void => {
            modified = true;
        });

        promise1.awaitSync();

        try {
            let value = JSRuntime.getPropertyInt(JSRuntime.getGlobal(), 'value');
            arktest.assertEQ(value, 7);
            arktest.assertEQ(modified, true);
        }
        catch(error) {
            console.log("Value modifications failed");
            return 1;
        }

        return 0;
    };

    let p = w.run<int>(callback);
    let result = await p;
    w.quit();
    arktest.assertEQ(result, 0);
}

async function CreateEAWorkerWithoutInterop(): Promise<void> {
    let eaw = new EAWorker();
    eaw.start();
    let res = false;
    try {
        let callback = () => {
            let module = JSRuntime.loadModule('./eaworker_test_helpers_js');
            let val = JSRuntime.getPropertyJSValue(module, 'returnTwo');
        };
        await eaw.run<void>(callback);
    } catch(e) {
        res = true;
        eaw.quit();
    }
    await eaw.join();
    arktest.assertEQ(res, true);
}

async function asyncCall(task: () => void): Promise<void> {
    task();
}

async function RunSetTimeoutTest(): Promise<void> {
    let w = new EAWorker(true);
    w.start();
    let resolver: ((val: Any) => void) | null = null;
    let done = new Promise<Any>((resolve) => {
        resolver = resolve;
    });
    let p = w.run<Any>(()=>{
        let p1 = asyncCall(() => {});

        p1.then<Any, Any>((value: Any) => {
            setTimeout(() => {
                resolver!(undefined);
            }, 10);
        });
    });
    await p;
    await done;
    w.quit();
}

async function RunNewCoroInSetTimeoutTest(): Promise<void> {
    let w = new EAWorker(true);
    w.start();
    let resolver: ((val: Any) => void) | null = null;
    let done = new Promise<Any>((resolve) => {
        resolver = resolve;
    });
    let p = w.run<Any>(()=>{
        let p1 = asyncCall(() => {
        });
        p1.then<Any, Any>((value: Any) => {
            setTimeout(() => {
                let p3 = asyncCall(()=>{
                    resolver!(undefined);
                });
            }, 10);
        });
    });
    await p;
    await done;
    w.quit();
}
