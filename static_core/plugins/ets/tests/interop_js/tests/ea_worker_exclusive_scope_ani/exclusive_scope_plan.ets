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

import { launch, waitForCompletion, LaunchParams, WorkerDomain, WorkerGroup } from 'std/concurrency';
import { CoroutineExtras } from 'std/debug/concurrency';

class ExclusivePayload {
    public value: int;
    public label: string;

    public constructor(value: int, label: string) {
        this.value = value;
        this.label = label;
    }
}

class ExclusiveCustomError extends Error {}

class ExclusiveScopeTestHooks {
    public static native waitUntilOsStackWorkReady(workerId: int): boolean;
}

class ExclusiveScopeStaticInitializer {
    public static value: int = 42;

    static {
        arktest.expectThrow((): void => {
            EAWorker.exclusiveScope<int>(() => 42);
        }, (error: Error): boolean | string => {
            return error instanceof InvalidJobOperationError ? true :
                'static initializer did not throw InvalidJobOperationError';
        });
    }
}

function throwManagedStackError(): never {
    throw new Error('managed stack stitching');
}

function requestManagedStackError(): Error {
    try {
        EAWorker.exclusiveScope<void>((): void => throwManagedStackError());
    } catch (error) {
        return error;
    }
    throw new Error('exclusiveScope callback did not throw');
}

function expectInvalidJobOperation(action: () => void, label: string): void {
    arktest.expectThrow(action, (error: Error): boolean | string => {
        return error instanceof InvalidJobOperationError ? true :
            label + ' did not throw InvalidJobOperationError';
    });
}

function expectRuntimeError(action: () => void, expectedMessage: string, label: string): void {
    arktest.expectThrow(action, (error: Error): boolean | string => {
        return error instanceof RuntimeError && error.message == expectedMessage ? true :
            label + ' expected RuntimeError: ' + expectedMessage +
                '; actual: ' + error.name + ': ' + error.message;
    });
}

function checkHealthyScope(): void {
    arktest.assertTrue(EAWorker.exclusiveScope<int>(() => 73) == 73,
        'exclusiveScope did not recover after a rejected switch');
}

function startInitializedInteropWorker(): EAWorker {
    let initialized = new CompletableJob<void>();
    let executorCoroutineId = new AtomicInt(-1);
    let worker = new EAWorker((): void => {
        executorCoroutineId.store(CoroutineExtras.getCoroutineId());
        initialized.finish();
    }, true);
    worker.start();
    initialized.Await();

    let workerId = worker.getWorkerId();
    if (workerId === undefined) {
        arktest.assertTrue(false, 'initialized EAWorker has no worker id');
        return worker;
    }
    arktest.assertTrue(ExclusiveScopeTestHooks.waitUntilOsStackWorkReady(workerId),
        'EAWorker OS-stack queue did not become ready');

    let scheduledProbe = worker.run<void>((): void => {
        arktest.assertTrue(CoroutineExtras.getCoroutineId() != executorCoroutineId.load(),
            'readiness barrier returned an executor-inline task');
    });
    scheduledProbe.Await();
    return worker;
}

export function TestInitialTaskInline(): void {
    let done = new CompletableJob<int>();
    let worker = new EAWorker((): void => {
        try {
            done.finish(EAWorker.exclusiveScope<int>(() => 1));
        } catch (error) {
            done.fail(error);
        }
    }, true);
    worker.start();
    let result = done.Await();
    worker.join().Await();
    arktest.assertTrue(result == 1, 'initial-task inline result mismatch');
}

export function TestInitialTaskChildRejected(): void {
    let done = new CompletableJob<void>();
    let worker = new EAWorker((): void => {
        try {
            let params: LaunchParams = {
                abortFlag: false,
                workerGroupId: WorkerGroup.generateGroupId(
                    WorkerDomain.EXACT_ID, [CoroutineExtras.getWorkerId()])
            };
            let child = launch<void>(params, (): void => {
                expectRuntimeError((): void => {
                    EAWorker.exclusiveScope<int>(() => 1);
                }, 'EAWorker:: exclusiveScope cannot be called before the initial task completes',
                    'initial-task child exclusiveScope');
            });
            child.Await();
            done.finish();
        } catch (error) {
            done.fail(error);
        }
    }, true);
    worker.start();
    done.Await();
    worker.join().Await();
}

export function TestReturnTypesAndSerialCalls(): void {
    let worker = startInitializedInteropWorker();
    let job = worker.run<void>((): void => {
        let text = EAWorker.exclusiveScope<string>(() => 'exclusive-string');
        arktest.assertTrue(text == 'exclusive-string', 'string result mismatch');

        let payload = EAWorker.exclusiveScope<ExclusivePayload>(
            () => new ExclusivePayload(42, 'exclusive-object'));
        arktest.assertTrue(payload.value == 42 && payload.label == 'exclusive-object',
            'object result mismatch');

        let boxed = EAWorker.exclusiveScope<Object>(() => 17);
        arktest.assertTrue((boxed as int) == 17, 'boxed primitive result mismatch');

        let nullable = EAWorker.exclusiveScope<ExclusivePayload | null>(() => null);
        arktest.assertTrue(nullable === null, 'null result mismatch');

        let sideEffect = new AtomicInt(0);
        EAWorker.exclusiveScope<void>(() => { sideEffect.store(9); });
        arktest.assertTrue(sideEffect.load() == 9, 'void callback did not execute');

        let sum = 0;
        for (let i = 1; i <= 20; ++i) {
            sum += EAWorker.exclusiveScope<int>(() => i);
        }
        arktest.assertTrue(sum == 210, 'serial exclusiveScope result mismatch');
    });
    job.Await();
    worker.join().Await();
}

export function TestExceptionIdentityAndRecovery(): void {
    let worker = startInitializedInteropWorker();
    let job = worker.run<void>((): void => {
        let expected = new ExclusiveCustomError();
        let same = false;
        try {
            EAWorker.exclusiveScope<void>(() => { throw expected; });
        } catch (error) {
            same = error === expected;
        }
        arktest.assertTrue(same, 'callback exception identity was not preserved');
        arktest.assertEQ(EAWorker.exclusiveScope<int>(() => 99), 99,
            'scope did not recover after callback exception');
    });
    job.Await();
    worker.join().Await();
}

export function TestManagedStackStitching(): void {
    let worker = startInitializedInteropWorker();
    let job = worker.run<string>((): string => {
        let error = requestManagedStackError();
        let stack = error.stack!;
        let executorIndex = stack.indexOf('throwManagedStackError');
        let requesterIndex = stack.indexOf('requestManagedStackError');
        arktest.assertTrue(executorIndex != -1,
            'managed stack does not contain executor frame');
        arktest.assertTrue(requesterIndex != -1,
            'managed stack does not contain requester frame');
        arktest.assertTrue(executorIndex < requesterIndex,
            'managed executor and requester frames are in the wrong order');
        return stack;
    });
    let stack = job.Await();
    worker.join().Await();
    arktest.assertTrue(stack.indexOf('throwManagedStackError') != -1 &&
        stack.indexOf('requestManagedStackError') != -1,
        'managed stack stitching result mismatch');
}

export function TestManagedStackStitchingDisabled(): void {
    let worker = startInitializedInteropWorker();
    let job = worker.run<string>((): string => requestManagedStackError().stack!);
    let stack = job.Await();
    worker.join().Await();
    arktest.assertTrue(stack.indexOf('throwManagedStackError') != -1,
        'disabled managed stack does not contain executor frame');
    arktest.assertTrue(stack.indexOf('requestManagedStackError') == -1,
        'disabled managed stack unexpectedly contains requester frame');
}

export function TestFifoAndIndependentResults(): void {
    const count = 16;
    let worker = startInitializedInteropWorker();
    let submitted = new AtomicInt(0);
    let executed = new AtomicInt(0);
    let jobs: Job<int>[] = [];
    for (let i = 0; i < count; ++i) {
        jobs.push(worker.run<int>((): int => {
            let ticket = submitted.fetchAdd(1);
            return EAWorker.exclusiveScope<int>((): int => {
                let position = executed.fetchAdd(1);
                return ticket == position ? ticket : -1000 - ticket;
            });
        }));
    }

    let seen: boolean[] = [];
    for (let i = 0; i < count; ++i) {
        seen.push(false);
    }
    for (let job of jobs) {
        let result = job.Await();
        arktest.assertTrue(result >= 0 && result < count, 'FIFO order mismatch');
        arktest.assertTrue(!seen[result], 'requester result was duplicated');
        seen[result] = true;
    }
    worker.join().Await();
    arktest.assertEQ(submitted.load(), count, 'not all scopes were submitted');
    arktest.assertEQ(executed.load(), count, 'not all queued scopes executed');
}

export function TestSynchronousInterop(): void {
    let worker = startInitializedInteropWorker();
    let job = worker.run<void>((): void => {
        let value = EAWorker.exclusiveScope<double>((): double => {
            let module = JSRuntime.loadModule('./exclusive_scope_helper');
            let add = JSRuntime.getPropertyJSValue(module, 'add');
            let sum = JSRuntime.invoke(module, add, 20, 22) as double;
            arktest.assertTrue(sum == 42.0, 'ETS -> JS return mismatch');
            return sum;
        });
        arktest.assertTrue(value == 42.0, 'exclusiveScope did not propagate the interop result');
        let caughtJs = false;
        try {
            EAWorker.exclusiveScope<void>((): void => {
                let module = JSRuntime.loadModule('./exclusive_scope_helper');
                let throwFromJs = JSRuntime.getPropertyJSValue(module, 'throwFromJs');
                JSRuntime.invoke(module, throwFromJs);
            });
        } catch (error) {
            caughtJs = true;
        }
        arktest.assertTrue(caughtJs, 'JS exception did not propagate through exclusiveScope');
        arktest.assertTrue(EAWorker.exclusiveScope<int>(() => 7) == 7,
            'scope did not recover after a JS exception');
    });
    job.Await();
    worker.join().Await();
}

export function TestJsEtsReentry(): void {
    let worker = startInitializedInteropWorker();
    let job = worker.run<void>((): void => {
        EAWorker.exclusiveScope<void>((): void => {
            let module = JSRuntime.loadModule('./exclusive_scope_helper');
            let callEts = JSRuntime.getPropertyJSValue(module, 'callEtsCallback');
            let reentered = ESValue.wrap(callEts).invoke((): number => 42).toNumber();
            arktest.assertTrue(reentered == 42.0, 'synchronous JS -> ETS reentry mismatch');
        });
    });
    job.Await();
    worker.join().Await();
}

export function TestGcRoots(): void {
    let worker = startInitializedInteropWorker();
    let job = worker.run<ExclusivePayload>((): ExclusivePayload => {
        let captured = new ExclusivePayload(41, 'captured-root');
        return EAWorker.exclusiveScope<ExclusivePayload>((): ExclusivePayload => {
            let garbage: ExclusivePayload[] = [];
            for (let i = 0; i < 20000; ++i) {
                garbage.push(new ExclusivePayload(i, 'garbage-' + i));
            }
            GC.startGC(GC.Cause.FULL, GC.IN_PLACE_MODE);
            arktest.assertTrue(captured.value == 41 && captured.label == 'captured-root',
                'captured callback root was corrupted after GC');
            return new ExclusivePayload(captured.value + 1, 'result-root');
        });
    });
    let result = job.Await();
    GC.startGC(GC.Cause.FULL, GC.IN_PLACE_MODE);
    arktest.assertTrue(result.value == 42 && result.label == 'result-root',
        'result root was corrupted after GC');
    worker.join().Await();
}

export function TestNoSwitchApis(): void {
    let worker = startInitializedInteropWorker();
    let job = worker.run<void>((): void => {
        expectInvalidJobOperation((): void => {
            EAWorker.exclusiveScope<void>(() => { launch<int>(() => 1); });
        }, 'launch');
        checkHealthyScope();

        let completed = new CompletableJob<int>();
        completed.finish(1);
        expectInvalidJobOperation((): void => {
            EAWorker.exclusiveScope<void>(() => { completed.Await(); });
        }, 'Job.Await');
        checkHealthyScope();

        expectInvalidJobOperation((): void => {
            EAWorker.exclusiveScope<void>(() => {
                waitForCompletion<int>(() => Promise.resolve<int>(1));
            });
        }, 'Promise await');
        checkHealthyScope();

        expectInvalidJobOperation((): void => {
            EAWorker.exclusiveScope<void>(() => { setTimeout(() => {}, 1); });
        }, 'timer creation');
        checkHealthyScope();

        expectInvalidJobOperation((): void => {
            EAWorker.exclusiveScope<void>(() => { Coroutine.Schedule(); });
        }, 'Coroutine.Schedule');
        checkHealthyScope();
    });
    job.Await();
    worker.join().Await();
}

export function TestStaticInitializerRejected(): void {
    let worker = startInitializedInteropWorker();
    let job = worker.run<void>((): void => {
        let value = ExclusiveScopeStaticInitializer.value;
        arktest.assertEQ(value, 42,
            'exclusiveScope in a static initializer did not throw InvalidJobOperationError');
        checkHealthyScope();
    });
    job.Await();
    worker.join().Await();
}

export function TestNestedRejected(): void {
    let worker = startInitializedInteropWorker();
    let job = worker.run<void>((): void => {
        EAWorker.exclusiveScope<void>((): void => {
            expectRuntimeError((): void => {
                EAWorker.exclusiveScope<void>(() => {});
            }, 'EAWorker:: nested exclusiveScope is not allowed', 'nested exclusiveScope');
        });
        arktest.assertEQ(EAWorker.exclusiveScope<int>(() => 1), 1,
            'scope did not recover after nested exclusiveScope rejection');
    });
    job.Await();
    worker.join().Await();
}

export function TestMainThreadRejected(): void {
    expectRuntimeError((): void => {
        EAWorker.exclusiveScope<int>(() => 1);
    }, 'EAWorker:: exclusiveScope can only be called in EAWorker', 'main-thread exclusiveScope');
}

export function TestGeneralWorkerRejected(): void {
    let job = launch<void>((): void => {
        expectRuntimeError((): void => {
            EAWorker.exclusiveScope<int>(() => 1);
        }, 'EAWorker:: exclusiveScope can only be called in EAWorker', 'general-worker exclusiveScope');
    });
    job.Await();
}

export function TestNonInteropWorkerRejected(): void {
    let worker = new EAWorker(false);
    worker.start();
    let job = worker.run<void>((): void => {
        expectRuntimeError((): void => {
            EAWorker.exclusiveScope<int>(() => 1);
        }, 'EAWorker:: exclusiveScope requires an EAWorker created with needInterop=true',
            'EAWorker(false) exclusiveScope');
    });
    job.Await();
    worker.join().Await();
}

function callExclusiveScopeFromTaskpool(): void {
    expectRuntimeError((): void => {
        EAWorker.exclusiveScope<int>(() => 1);
    }, 'EAWorker:: exclusiveScope is not supported in taskpool workers', 'taskpool exclusiveScope');
}

export function TestTaskpoolRejected(): void {
    try {
        waitForCompletion<Any>(() => taskpool.execute(callExclusiveScopeFromTaskpool));
    } finally {
        CoroutineExtras.stopTaskpool();
    }
}

export function TestShutdownDrainAndReject(): void {
    let worker = startInitializedInteropWorker();
    let callbackStarted = new CompletableJob<void>();
    let releaseCallback = new AtomicInt(0);
    let acceptedJob = worker.run<int>((): int => {
        return EAWorker.exclusiveScope<int>((): int => {
            callbackStarted.finish();
            while (releaseCallback.load() == 0) {
                // Deliberately stay on the executor OS stack without a coroutine switch.
            }
            return 42;
        });
    });
    callbackStarted.Await();
    let joinJob = worker.join();
    releaseCallback.store(1);
    arktest.assertTrue(acceptedJob.Await() == 42, 'accepted shutdown task was not drained');
    joinJob.Await();

    let rejectWorker = startInitializedInteropWorker();
    let rejectEntered = new CompletableJob<void>();
    let rejectGate = new CompletableJob<void>();
    let rejectedJob = rejectWorker.run<void>((): void => {
        rejectEntered.finish();
        rejectGate.Await();
        expectRuntimeError((): void => {
            EAWorker.exclusiveScope<int>(() => 1);
        }, 'EAWorker:: exclusiveScope cannot be started while the worker is shutting down',
            'exclusiveScope submitted after worker close');
    });
    rejectEntered.Await();
    let rejectJoin = rejectWorker.join();
    rejectGate.finish();
    rejectedJob.Await();
    rejectJoin.Await();
}

export function TestReturnExceptionStress(): void {
    const count = 100;
    let worker = startInitializedInteropWorker();
    let job = worker.run<int>((): int => {
        let sum = 0;
        for (let i = 0; i < count; ++i) {
            if (i % 3 != 0) {
                arktest.assertTrue(EAWorker.exclusiveScope<int>(() => i) == i,
                    'stress result state leaked');
            } else {
                let expected = new Error('stress-' + i);
                let same = false;
                try {
                    EAWorker.exclusiveScope<void>(() => { throw expected; });
                } catch (error) {
                    same = error === expected;
                }
                arktest.assertTrue(same, 'stress exception state leaked');
            }
            sum += i;
        }
        return sum;
    });
    let sum = job.Await();
    worker.join().Await();
    arktest.assertTrue(sum == 4950, 'stress checksum mismatch');
}

function runAllFromEtsMain(): void {
    TestInitialTaskInline();
    TestInitialTaskChildRejected();
    TestReturnTypesAndSerialCalls();
    TestExceptionIdentityAndRecovery();
    TestManagedStackStitching();
    TestSynchronousInterop();
    TestJsEtsReentry();
    TestGcRoots();
    TestNoSwitchApis();
    TestStaticInitializerRejected();
    TestNestedRejected();
    TestMainThreadRejected();
    TestGeneralWorkerRejected();
    TestNonInteropWorkerRejected();
    TestTaskpoolRejected();
    TestFifoAndIndependentResults();
    TestShutdownDrainAndReject();
    TestReturnExceptionStress();
}

export class ExclusiveScopePlanRunner {
    public static testManagedStackStitchingDisabled(): void {
        TestManagedStackStitchingDisabled();
    }

    public static main(): void {
        runAllFromEtsMain();
    }
}
