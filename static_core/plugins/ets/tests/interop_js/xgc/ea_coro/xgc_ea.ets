/**
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

package xgc_ea

const SHARED_OBJECTS_COUNT: int = 10;

class SharedClass {
    public jsRef?: JSValue;
}

const sharedObjects: FixedArray<(WeakRef<SharedClass>|undefined)>;
static {
    sharedObjects = new undefined[SHARED_OBJECTS_COUNT];
    for (let i: int = 0; i < SHARED_OBJECTS_COUNT; ++i) {
        sharedObjects[i] = new WeakRef<SharedClass>(new SharedClass());
    }
}


function xgc(): void throws {
    GC.waitForFinishGC(JSRuntime.xgc());
}

function runFullGC(): void throws {
    GC.waitForFinishGC(GC.startGC(GC.Cause.FULL));
}

// If cross reference is not existed then create it: JS object -> ETS object
function getSharedObject(index: int): SharedClass|undefined throws {
    return sharedObjects[index]!.deref();
}

// Create refs: ETS object -> JSValue (ETS object) -> JS object
function setRefToSharedObject(index: int, jsRef: JSValue): void throws {
    sharedObjects[index]!.deref()!.jsRef = jsRef;
}
