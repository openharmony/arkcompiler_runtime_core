/**
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

let workerId = -1;

function setWorkerId(eworkerId: int)
{
    workerId = eworkerId;
}

function checkWorkerId(): boolean
{
    return CoroutineExtras.getWorkerId() == workerId;
}

// CallMethod
function call() {
    return checkWorkerId();
}

// AsyncCallMethod
function asyncCall(): boolean {
    let l = async (): Promise<boolean> => {
        return Promise.resolve(call());
    }
    return await l();
}

// LaunchCallMethod
function launchCall(): boolean {
    let p = launch call();
    return await p;
}

// ConcurrentWorkerAndRuntimeDestroy
class Event {
    constructor() {
        this.promise = new Promise<Number>((resolve: (val: Number) => void) => {
            this.resolveFn = resolve;
        })
    }

    public wait() {
        await this.promise;
    }

    public fire() {
        this.resolveFn!(0);
    }

    private promise: Promise<Number>;
    private resolveFn: ((val: Number) => void) | null = null;
}

let mainEvent = new Event();
let eWorkerAsyncEvent = new Event();
let eWorkerLaunchEvent = new Event();

function mainRoutine() {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    launch () => {
        eWorkerLaunchEvent.wait();
        mainEvent.fire();
    }();
}

function eWorkerRoutine(): boolean {
    async () => {
        eWorkerAsyncEvent.wait();
    }();

    launch () => {
        eWorkerLaunchEvent.fire();
        mainEvent.wait();
        eWorkerAsyncEvent.fire();
    }();
    return true;
}
