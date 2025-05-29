/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

function main(): int {
    let asyncLockTestsuite = new ArkTestsuite("asyncLockTestsuite");

    asyncLockTestsuite.addTest("TestWithEqualityAndNonEquality", () => {
        let failures = 0;
        failures += asyncLockQueryTest();
        failures += asyncLockDeadLockTest();
        failures += asyncLockAvailableTest();
        failures += asyncLockExclusiveModeTest();
        failures += asyncLockQueryNonexistentNameTest();

        assertEQ(failures, 0);
    });

    return asyncLockTestsuite.run();
}

/**
 *  Check if lock returns information about current execution state.
 */
function asyncLockQueryTest(): int {
    let lock = AsyncLock.request("asyncLockQueryTest");
    let promise = lock.lockAsync((): int => {
        let state = lock.query();
        return state.held.length as int;
    });

    let result = await promise;
    if (result == 1) {
        return 0;
    } else {
        StdDebug.Logger.log("FAILED: AsyncLock query test");
        return 1;
    }
}

/**
 *  Check if the query exception causes deadlock.
 */
function asyncLockQueryNonexistentNameTest(): int {
    let result = 0;
    let lockname = "name";
    try {
        AsyncLock.query(lockname);
    } catch (e: Error) {
        if(e.message != "AsyncLock " + lockname + " not found") {
            result += 1; 
        }
    }

    AsyncLock.request(lockname);

    if (result != 0) {
        StdDebug.Logger.log("FAILED: AsyncLock query nonexistent name test");
    }

    return result;
}

/**
 *  Check if async lock survives dead lock.
 */
function asyncLockDeadLockTest(): int {
    let lock = new AsyncLock();

    let promise = lock.lockAsync((): int => {
        try {
            await lock.lockAsync((): int => {
                return 0;
            }, AsyncLockMode.EXCLUSIVE, new AsyncLockOptions(false, null, 1));
        } catch (err: Error) {
            return 0;
        }
        StdDebug.Logger.log("FAILED: AsyncLock dead lock test");
        return 1;
    });

    return await promise;
}

/**
*  Check if callback can be interupted because of isAvailable.
*/
function asyncLockAvailableTest(): int {
    let lock = new AsyncLock();
    let options = new AsyncLockOptions(true, null, 0)

    let promise = lock.lockAsync((): int => {
        try {
            await lock.lockAsync((): int => {
                return 0;
            }, AsyncLockMode.EXCLUSIVE, options);
        } catch (err: Error) {
            return 0;
        }
        StdDebug.Logger.log("FAILED: AsyncLock isAvailable test");
        return 1;
    }, AsyncLockMode.EXCLUSIVE, options);

    return await promise;
}

function testCallbackExclusive(lock: AsyncLock): Promise<int> {

    let promise = lock.lockAsync((): int => {
        let status = lock.query();
        return status.held.length == 1 ? 0 : 1;
    }, AsyncLockMode.EXCLUSIVE);
    return promise;
}

/**
 *  Check if in EXCLUSIVE mode callbacks execute one by one for any caller.
 */
function asyncLockExclusiveModeTest(): int {

    let lock = new AsyncLock();

    let promise1 = launch testCallbackExclusive(lock);
    let promise2 = launch testCallbackExclusive(lock);
    let promise3 = testCallbackExclusive(lock);
    let promise4 = testCallbackExclusive(lock);

    let result = 0;
    result += await promise4;
    result += await promise3;
    result += await promise2;
    result += await promise1;

    if (result != 0) {
        StdDebug.Logger.log("FAILED: AsyncLock EXCLUSIVE mode test");
    }

    return result;
}
