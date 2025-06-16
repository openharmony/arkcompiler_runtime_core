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
        let method = JSRuntime.getPropertyJSValue(module, 'returnTwo');
        let val = JSRuntime.invoke(module, method);
        try {
            arktest.assertEQ(JSRuntime.getValueDouble(val) as int, 2);
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
    arktest.assertEQ(result1, 0);
    arktest.assertEQ(result2, 0);
}

function RunTasksWithJsAsyncCallTest(): void {
    let callback = ():int => {
        let module = JSRuntime.loadModule("./taskpool_interop_tests_helpers_js");
        let modified = false;
        let method = JSRuntime.getPropertyJSValue(module, 'asyncWithAwait');
        let callRes = JSRuntime.invoke(module, method);
        let linker = Class.ofCaller()!.getLinker();
        let clsName = 'std.core.Promise';
        let promise = JSRuntime.getValueObject(callRes, linker.loadClass(clsName, false)) as Promise<Object>;
        let promise1 = promise.then((x:Object): void => {
            modified = true;
        });

        await promise1;

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

    let p = taskpool.execute(callback);
    let result = await p;
    arktest.assertEQ(result, 0);
    arktest.assertEQ(CoroutineExtras.isTaskpoolSupportingInterop(), true);
    CoroutineExtras.stopTaskpool();
}
