/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

import { launch } from 'std/concurrency'
import { CoroutineDump } from 'std/dfx'

let arr = new Int32Array(1);
let mutex = new Mutex();
let rwLock = new RWLock();
const MANY_MUTEX_WAITER_COUNT: int = 64;

async function waiter() {
    console.log('start');
    await Atomics.waitAsync(arr, 0, 0);
    console.log('end');
}

function startWaitCoro(): int {
    console.log('start wait coro');
    launch(waiter);
    return 0;
}

function notify(): int {
    Atomics.notify(arr, 0);
    return 0;
}

async function mutexWaiter() {
    mutex.lock();
    mutex.unlock();
}

function startMutexWaitCoro(): int {
    mutex.lock();
    launch(mutexWaiter);
    return 0;
}

function startManyMutexWaitCoroutines(): int {
    mutex.lock();
    for (let i: int = 0; i < MANY_MUTEX_WAITER_COUNT; ++i) {
        launch(mutexWaiter);
    }
    return 0;
}

function unlockMutex(): int {
    mutex.unlock();
    return 0;
}

async function rwWriteWaiter() {
    rwLock.writeLock().lock();
    rwLock.writeLock().unlock();
}

function startRWWriteWaitCoro(): int {
    rwLock.readLock().lock();
    launch(rwWriteWaiter);
    return 0;
}

function unlockRWRead(): int {
    rwLock.readLock().unlock();
    return 0;
}

async function rwReadWaiter() {
    rwLock.readLock().lock();
    rwLock.readLock().unlock();
}

function startRWReadWaitCoro(): int {
    rwLock.writeLock().lock();
    launch(rwReadWaiter);
    return 0;
}

function unlockRWWrite(): int {
    rwLock.writeLock().unlock();
    return 0;
}

function dumpContainsMutexWait(): int {
    const dump = CoroutineDump.dumpCoroutineInfo();
    for (let index: int = 0; index < dump.length; ++index) {
        const info = dump[index]
        if (info.waitReason == 'MUTEX_LOCK' &&
            info.lockMode == 'MUTEX' &&
            info.status == 'BLOCKED' &&
            info.type == 'MUTATOR' &&
            info.coroutineName.includes('mutexWaiter') &&
            info.callStack.includes('Mutex::lock')) {
            return 1
        }
    }
    return 0
}

function dumpContainsAllMutexWaiters(): int {
    const dump = CoroutineDump.dumpCoroutineInfo();
    let blockedMutexWaiters: int = 0
    for (let index: int = 0; index < dump.length; ++index) {
        if (dump[index].waitReason == 'MUTEX_LOCK') {
            ++blockedMutexWaiters
        }
    }
    return blockedMutexWaiters >= MANY_MUTEX_WAITER_COUNT ? 1 : 0
}

function dumpContainsRWWriteWait(): int {
    const dump = CoroutineDump.dumpCoroutineInfo()
    for (let index: int = 0; index < dump.length; ++index) {
        const info = dump[index]
        if (info.waitReason == 'RWLOCK_WRITE' &&
            info.lockMode == 'WRITELOCK' &&
            info.readerCount == 1 && info.writerCount == 1) {
            return 1
        }
    }
    return 0
}

function dumpContainsRWReadWait(): int {
    const dump = CoroutineDump.dumpCoroutineInfo()
    for (let index: int = 0; index < dump.length; ++index) {
        const info = dump[index]
        if (info.waitReason == 'RWLOCK_READ' &&
            info.lockMode == 'READLOCK' &&
            info.readerCount == 1 && info.writerCount == 1) {
            return 1
        }
    }
    return 0
}
