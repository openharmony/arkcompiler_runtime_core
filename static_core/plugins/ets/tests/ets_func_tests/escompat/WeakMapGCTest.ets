/*
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

class Key {
    readonly id: int

    constructor(id: int) {
        this.id = id
    }
}

class Val {
    readonly id: int

    constructor(id: int) {
        this.id = id
    }
}

const NULL_ID = 0
let reclaimedKeyId = NULL_ID

const keyRegistry = new FinalizationRegistry<Int>((keyId: Int) => {
    reclaimedKeyId = keyId
})

function isStwGcType(): boolean {
    try {
        // using the fact that startGC(YOUNG_CAUSE) request throws when GC type is STW
        GC.startGC(GC.YOUNG_CAUSE)
    } catch (gcEx) {
        return true
    }

    return false
}

const STW_GC: boolean = isStwGcType()

function createTestWeakMap(keyId: int): WeakMap<Key, Val> {
    const map = new WeakMap<Key, Val>()

    if (keyId != NULL_ID) {
        let key: Key | null = new Key(keyId)
        map.set(key, new Val(keyId))

        keyRegistry.register(key, keyId)
        key = null
    }

    return map
}

function weakMapKeyGarbageCollection(): void {
    if (STW_GC) {
        // non-production scenario
        return
    }

    const KEY_ID = 2
    reclaimedKeyId = NULL_ID

    // using coroutine to avoid WeakMap creation function inlining
    const weakMapCreator = launch createTestWeakMap(KEY_ID)
    const weakMap = await weakMapCreator

    while (reclaimedKeyId != KEY_ID) {
        try {
            const gcId = GC.startGC(GC.FULL_CAUSE, GC.IN_PLACE_MODE)
            // We need to switch from main coroutine to coroutine created by FinalizationRegistry because 
            // if main coro is executed first, the test will return false
            Coroutine.Schedule()
            if (gcId > 0) {
                GC.waitForFinishGC(gcId)
            }
        } catch (gcEx) {
            assertTrue(false)
        }
    }
}

function main(): int {
    const suite = new ArkTestsuite("WeakMap GC tests")

    suite.addTest("WeakMap keys garbage collection", weakMapKeyGarbageCollection)

    return suite.run()
}
