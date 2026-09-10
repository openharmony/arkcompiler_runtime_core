/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

class CoroutineSignal {
    private promise: Promise<int>;
    private resolver: ((value: int) => void) | null = null;

    constructor() {
        this.promise = new Promise<int>((resolve: (value: int) => void) => {
            this.resolver = resolve;
        });
    }

    public wait(): int {
        return this.promise.awaitResolution();
    }

    public signal(): void {
        this.resolver!(1);
    }
}

let aEntered = new CoroutineSignal();
let resumeA = new CoroutineSignal();
let resumeB = new CoroutineSignal();

native function captureActiveInteropStack(): void

native function assertActiveInteropStack(): void

function invokeAndVerify(callback: () => void): boolean {
    let module = JSRuntime.loadModule('./interop_stack_info_helpers');
    let method = JSRuntime.getPropertyJSValue(module, 'invokeAndVerify');
    return JSRuntime.invoke(module, method, callback) as boolean;
}

function runScopeA(): boolean {
    // Keep A's real ETS-to-JS call open until B has entered its own call.
    let result = invokeAndVerify((): void => {
        aEntered.signal();
        resumeA.wait();
    });

    // Scope A has closed before B is resumed.
    resumeB.signal();
    return result;
}

function runScopeB(): boolean {
    aEntered.wait();

    // Wake A from B's JS callback, then wait until A has closed its call.
    let result = invokeAndVerify((): void => {
        // Save B's boundary while B owns the ETS-to-JS scope.
        captureActiveInteropStack();
        resumeA.signal();
        resumeB.wait();

        // B must regain its own boundary before returning to JS.
        assertActiveInteropStack();
    });
    return result;
}

export function runOverlappingScopeTest(): void {
    aEntered = new CoroutineSignal();
    resumeA = new CoroutineSignal();
    resumeB = new CoroutineSignal();

    // Reuse the JS-to-ETS entry worker so A and B share the launcher's napi_env.
    let worker = EAWorker.main();
    let resultA = worker.run<boolean>(runScopeA);
    let resultB = worker.run<boolean>(runScopeB);

    arktest.assertEQ(resultA.Await(), true);
    arktest.assertEQ(resultB.Await(), true);
}
