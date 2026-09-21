/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

import { launch } from "std/concurrency";
import { CompletableJob, Job } from "std/core";
import { AtomicFlag } from "std/debug/concurrency";

native function observeAsyncStack(value: int): int;
native function inspectPromiseQueue(promise: Object): int;
native function recordSubmissionWorker(): int;

let pendingPromise: Promise<void> | undefined = undefined;
let pendingContinuation: Promise<void> | undefined = undefined;
let pendingResolve: ((value: PromiseLike<undefined> | undefined) => void) | undefined = undefined;
let pendingRejectContinuation: Promise<void> | undefined = undefined;
let pendingReject: ((error: Error) => void) | undefined = undefined;
let pendingFinallyContinuation: Promise<void> | undefined = undefined;
let pendingFinallyResolve: ((value: PromiseLike<undefined> | undefined) => void) | undefined = undefined;
let awaitContinuation: Promise<void> | undefined = undefined;
let awaitFirstResolve: ((value: PromiseLike<undefined> | undefined) => void) | undefined = undefined;
let awaitSecondResolve: ((value: PromiseLike<undefined> | undefined) => void) | undefined = undefined;
let awaitThirdResolve: ((value: PromiseLike<undefined> | undefined) => void) | undefined = undefined;
let awaitObservationResolve: ((value: PromiseLike<undefined> | undefined) => void) | undefined = undefined;
let workerAwaitJob: Job<Promise<void>> | undefined = undefined;
let workerAwaitResolve: ((value: PromiseLike<undefined> | undefined) => void) | undefined = undefined;
let workerStarted: CompletableJob<void> | undefined = undefined;
let workerThenResolve: ((value: PromiseLike<undefined> | undefined) => void) | undefined = undefined;
let workerSubmissionJob: Job<void> | undefined = undefined;
let workerSubmissionRelease: AtomicFlag = new AtomicFlag(false);
let nestedContinuation: Promise<void> | undefined = undefined;
let nestedOuterResolve: ((value: PromiseLike<undefined> | undefined) => void) | undefined = undefined;
let nestedInnerResolve: ((value: PromiseLike<undefined> | undefined) => void) | undefined = undefined;
let preAwaitOuterContinuation: Promise<void> | undefined = undefined;
let preAwaitInnerContinuation: Promise<void> | undefined = undefined;
let preAwaitOuterResolve: ((value: PromiseLike<undefined> | undefined) => void) | undefined = undefined;
let preAwaitInnerResolve: ((value: PromiseLike<undefined> | undefined) => void) | undefined = undefined;
let preAwaitThenContinuation: Promise<void> | undefined = undefined;
let preAwaitThenResolve: ((value: PromiseLike<undefined> | undefined) => void) | undefined = undefined;
let preAwaitCContinuation: Promise<void> | undefined = undefined;
let preAwaitCResolve: ((value: PromiseLike<undefined> | undefined) => void) | undefined = undefined;
let maxFramesContinuation: Promise<void> | undefined = undefined;

function alreadySettled(): int {
    waitForCompletion((): Promise<void> => Promise.resolve().then<void>((): void => {
        observeAsyncStack(1);
    }));
    return 0;
}

function alreadyRejectedCatch(): int {
    waitForCompletion((): Promise<void> => Promise.reject().catch<void>((): void => {
        observeAsyncStack(14);
    }));
    return 0;
}

function alreadySettledFinally(): int {
    waitForCompletion((): Promise<void> => Promise.resolve().finally((): void => {
        observeAsyncStack(15);
    }));
    return 0;
}

function submitPending(): int {
    let promise = new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void,
            reject: (error: Error) => void): void => {
        pendingResolve = resolve;
    });
    pendingPromise = promise;
    pendingContinuation = promise.then<void>((): void => {
        observeAsyncStack(2);
    });
    return 0;
}

function pendingQueueHasSnapshot(): boolean {
    let promise = pendingPromise;
    if (promise == undefined) {
        return false;
    }
    return inspectPromiseQueue(promise) == 15;
}

function resolvePending(): int {
    let resolve = pendingResolve;
    if (resolve != undefined) {
        resolve(undefined);
    }
    pendingResolve = undefined;
    let continuation = pendingContinuation;
    if (continuation != undefined) {
        waitForCompletion((): Promise<void> => continuation);
    }
    pendingContinuation = undefined;
    return 0;
}

function submitPendingCatch(): int {
    let promise = new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void,
            reject: (error: Error) => void): void => {
        pendingReject = reject;
    });
    pendingRejectContinuation = promise.catch<void>((): void => {
        observeAsyncStack(16);
    });
    return 0;
}

function rejectPendingCatch(): int {
    let reject = pendingReject;
    if (reject != undefined) {
        reject(new Error("expected rejection"));
    }
    pendingReject = undefined;

    let continuation = pendingRejectContinuation;
    if (continuation != undefined) {
        waitForCompletion((): Promise<void> => continuation);
    }
    pendingRejectContinuation = undefined;
    return 0;
}

function submitPendingFinally(): int {
    let promise = new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void,
            reject: (error: Error) => void): void => {
        pendingFinallyResolve = resolve;
    });
    pendingFinallyContinuation = promise.finally((): void => {
        observeAsyncStack(17);
    });
    return 0;
}

function resolvePendingFinally(): int {
    let resolve = pendingFinallyResolve;
    if (resolve != undefined) {
        resolve(undefined);
    }
    pendingFinallyResolve = undefined;

    let continuation = pendingFinallyContinuation;
    if (continuation != undefined) {
        waitForCompletion((): Promise<void> => continuation);
    }
    pendingFinallyContinuation = undefined;
    return 0;
}

function submitAwaitFirst(): int {
    let promise = new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void,
            reject: (error: Error) => void): void => {
        awaitFirstResolve = resolve;
    });
    awaitContinuation = (async (): Promise<void> => {
        await promise;
        observeAsyncStack(3);
    })();
    return 0;
}

function resolveAwaitFirst(): int {
    let resolve = awaitFirstResolve;
    if (resolve != undefined) {
        resolve(undefined);
    }
    awaitFirstResolve = undefined;

    let continuation = awaitContinuation;
    if (continuation != undefined) {
        waitForCompletion((): Promise<void> => continuation);
    }
    awaitContinuation = undefined;
    return 0;
}

async function awaitWithObservationBeforeFirstAwait(): Promise<void> {
    let promise = new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void,
            reject: (error: Error) => void): void => {
        awaitObservationResolve = resolve;
    });
    observeAsyncStack(18);
    await promise;
    observeAsyncStack(19);
}

function submitAwaitObservationBeforeFirstAwait(): int {
    awaitContinuation = awaitWithObservationBeforeFirstAwait();
    return 0;
}

function resolveAwaitObservationBeforeFirstAwait(): int {
    let resolve = awaitObservationResolve;
    if (resolve != undefined) {
        resolve(undefined);
    }
    awaitObservationResolve = undefined;

    let continuation = awaitContinuation;
    if (continuation != undefined) {
        waitForCompletion((): Promise<void> => continuation);
    }
    awaitContinuation = undefined;
    return 0;
}

function submitAwaitChain(): int {
    let firstPromise = new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void,
            reject: (error: Error) => void): void => {
        awaitFirstResolve = resolve;
    });
    let secondPromise = new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void,
            reject: (error: Error) => void): void => {
        awaitSecondResolve = resolve;
    });
    awaitContinuation = (async (): Promise<void> => {
        await firstPromise;
        observeAsyncStack(4);
        await secondPromise;
        observeAsyncStack(5);
    })();
    return 0;
}

function resolveAwaitChainFirst(): int {
    let resolve = awaitFirstResolve;
    if (resolve != undefined) {
        resolve(undefined);
    }
    awaitFirstResolve = undefined;
    waitForCompletion((): Promise<void> => {
        return new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void): void => {
            setTimeout((): void => {
                resolve(undefined);
            }, 0);
        });
    });
    return 0;
}

function resolveAwaitChainSecond(): int {
    let resolve = awaitSecondResolve;
    if (resolve != undefined) {
        resolve(undefined);
    }
    awaitSecondResolve = undefined;

    let continuation = awaitContinuation;
    if (continuation != undefined) {
        waitForCompletion((): Promise<void> => continuation);
    }
    awaitContinuation = undefined;
    return 0;
}

function submitAwaitGenerationMismatch(): int {
    let firstPromise = new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void,
            reject: (error: Error) => void): void => {
        awaitFirstResolve = resolve;
    });
    let secondPromise = new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void,
            reject: (error: Error) => void): void => {
        awaitSecondResolve = resolve;
    });
    let thirdPromise = new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void,
            reject: (error: Error) => void): void => {
        awaitThirdResolve = resolve;
    });
    awaitContinuation = (async (): Promise<void> => {
        await firstPromise;
        observeAsyncStack(6);
        await secondPromise;
        observeAsyncStack(7);
        await thirdPromise;
        observeAsyncStack(8);
    })();
    return 0;
}

function resolveAwaitGenerationSecond(): int {
    let resolve = awaitSecondResolve;
    if (resolve != undefined) {
        resolve(undefined);
    }
    awaitSecondResolve = undefined;
    waitForCompletion((): Promise<void> => {
        return new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void): void => {
            setTimeout((): void => {
                resolve(undefined);
            }, 0);
        });
    });
    return 0;
}

function resolveAwaitGenerationThird(): int {
    let resolve = awaitThirdResolve;
    if (resolve != undefined) {
        resolve(undefined);
    }
    awaitThirdResolve = undefined;

    let continuation = awaitContinuation;
    if (continuation != undefined) {
        waitForCompletion((): Promise<void> => continuation);
    }
    awaitContinuation = undefined;
    return 0;
}

function startAwaitOnWorker(): Promise<void> {
    let promise = new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void,
            reject: (error: Error) => void): void => {
        workerAwaitResolve = resolve;
    });
    let started = workerStarted;
    if (started != undefined) {
        started.finish(undefined);
    }
    return (async (): Promise<void> => {
        await promise;
        observeAsyncStack(9);
    })();
}

function submitAwaitOnWorker(): int {
    workerStarted = new CompletableJob<void>();
    workerAwaitJob = launch(startAwaitOnWorker);
    let started = workerStarted;
    if (started == undefined) {
        return 1;
    }
    started.Await();
    workerStarted = undefined;
    return 0;
}

function resolveAwaitOnWorker(): int {
    let resolve = workerAwaitResolve;
    if (resolve != undefined) {
        resolve(undefined);
    }
    workerAwaitResolve = undefined;

    let job = workerAwaitJob;
    if (job != undefined) {
        waitForCompletion((): Promise<void> => job.Await());
    }
    workerAwaitJob = undefined;
    return 0;
}

function startThenOnWorker(): void {
    recordSubmissionWorker();
    workerSubmissionRelease.set(false);
    let promise = new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void,
            reject: (error: Error) => void): void => {
        workerThenResolve = resolve;
    });
    promise.then<void>((): void => {
        observeAsyncStack(13);
    });
    let started = workerStarted;
    if (started != undefined) {
        started.finish(undefined);
    }
    while (!workerSubmissionRelease.get()) {
    }
}

function submitThenOnWorker(): int {
    workerSubmissionRelease.set(false);
    workerStarted = new CompletableJob<void>();
    workerSubmissionJob = launch(startThenOnWorker);
    let started = workerStarted;
    if (started == undefined) {
        return 1;
    }
    started.Await();
    workerStarted = undefined;
    return 0;
}

function resolveThenOnWorker(): int {
    let thenResolve = workerThenResolve;
    if (thenResolve != undefined) {
        thenResolve(undefined);
    }
    workerThenResolve = undefined;

    workerSubmissionRelease.set(true);

    let submissionJob = workerSubmissionJob;
    if (submissionJob != undefined) {
        submissionJob.Await();
    }
    workerSubmissionJob = undefined;
    return 0;
}

async function nestedInnerAwait(): Promise<void> {
    let promise = new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void,
            reject: (error: Error) => void): void => {
        nestedInnerResolve = resolve;
    });
    await promise;
    observeAsyncStack(10);
}

async function nestedOuterAwait(): Promise<void> {
    let promise = new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void,
            reject: (error: Error) => void): void => {
        nestedOuterResolve = resolve;
    });
    await promise;
    observeAsyncStack(11);
    await nestedInnerAwait();
    observeAsyncStack(12);
}

function submitNestedAwait(): int {
    nestedContinuation = nestedOuterAwait();
    return 0;
}

function resolveNestedOuterAwait(): int {
    let resolve = nestedOuterResolve;
    if (resolve != undefined) {
        resolve(undefined);
    }
    nestedOuterResolve = undefined;
    waitForCompletion((): Promise<void> => {
        return new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void): void => {
            setTimeout((): void => {
                resolve(undefined);
            }, 0);
        });
    });
    return 0;
}

function resolveNestedInnerAwait(): int {
    let resolve = nestedInnerResolve;
    if (resolve != undefined) {
        resolve(undefined);
    }
    nestedInnerResolve = undefined;

    let continuation = nestedContinuation;
    if (continuation != undefined) {
        waitForCompletion((): Promise<void> => continuation);
    }
    nestedContinuation = undefined;
    return 0;
}

async function preAwaitInner(): Promise<void> {
    let innerPromise = new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void,
            reject: (error: Error) => void): void => {
        preAwaitInnerResolve = resolve;
    });
    observeAsyncStack(20);
    await innerPromise;
    observeAsyncStack(21);

    let thenPromise = new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void,
            reject: (error: Error) => void): void => {
        preAwaitThenResolve = resolve;
    });
    preAwaitThenContinuation = thenPromise.then<void>(preAwaitThenCallback);
}

function preAwaitThenCallback(): void {
    preAwaitCContinuation = preAwaitC();
}

async function preAwaitC(): Promise<void> {
    observeAsyncStack(22);
    let promise = new Promise<void>((resolve: (value: PromiseLike<undefined> | undefined) => void,
            reject: (error: Error) => void): void => {
        preAwaitCResolve = resolve;
    });
    await promise;
    observeAsyncStack(23);
}

async function preAwaitOuter(): Promise<void> {
    preAwaitInnerContinuation = preAwaitInner();
    let inner = preAwaitInnerContinuation;
    if (inner == undefined) {
        return;
    }
    await inner;
}

function submitPreAwaitDeepChain(): int {
    preAwaitOuterContinuation = preAwaitOuter();
    return 0;
}

function resolvePreAwaitInner(): int {
    let resolve = preAwaitInnerResolve;
    if (resolve != undefined) {
        resolve(undefined);
    }
    preAwaitInnerResolve = undefined;

    let inner = preAwaitInnerContinuation;
    if (inner != undefined) {
        waitForCompletion((): Promise<void> => inner);
    }
    preAwaitInnerContinuation = undefined;

    let outer = preAwaitOuterContinuation;
    if (outer != undefined) {
        waitForCompletion((): Promise<void> => outer);
    }
    preAwaitOuterContinuation = undefined;
    return 0;
}

function resolvePreAwaitThen(): int {
    let resolve = preAwaitThenResolve;
    if (resolve != undefined) {
        resolve(undefined);
    }
    preAwaitThenResolve = undefined;

    let continuation = preAwaitThenContinuation;
    if (continuation != undefined) {
        waitForCompletion((): Promise<void> => continuation);
    }
    preAwaitThenContinuation = undefined;
    return 0;
}

function resolvePreAwaitC(): int {
    let resolve = preAwaitCResolve;
    if (resolve != undefined) {
        resolve(undefined);
    }
    preAwaitCResolve = undefined;

    let continuation = preAwaitCContinuation;
    if (continuation != undefined) {
        waitForCompletion((): Promise<void> => continuation);
    }
    preAwaitCContinuation = undefined;
    return 0;
}

async function maxFramesTarget(): Promise<void> {
    observeAsyncStack(24);
}

function maxFramesCaller(): int {
    maxFramesContinuation = maxFramesTarget();
    return 0;
}

function submitMaxFramesChain(): int {
    return maxFramesCaller();
}

function waitMaxFramesChain(): int {
    let continuation = maxFramesContinuation;
    if (continuation != undefined) {
        waitForCompletion((): Promise<void> => continuation);
    }
    maxFramesContinuation = undefined;
    return 0;
}
