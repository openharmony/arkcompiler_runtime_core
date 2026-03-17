/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

function createWeakRefObject(): WeakRef<Object> {
    let obj: Object = {};
    return new WeakRef<Object>(obj);
}

function createWeakRefForConcurrent(): WeakRef<Object> {
    let hugeArraySize: int = 100000;
    let obj = new long[hugeArraySize](0);
    return new WeakRef<Object>(obj);
}

function createWeakRefString(s: string): WeakRef<String> {
    let obj = 'Hello ' + s;
    return new WeakRef<String>(obj);
}

function createWeakRefStringForConcurrent(): WeakRef<String> {
    let obj = 'Hello';
    for (let i = 0; i < 20; ++i) {
        obj += obj;
    }
    return new WeakRef<String>(obj);
}

// Make it global because optimizer is free to kill local variables
const reachableObj: Object = {};

function fullGCWeakRefsMarkingCheck(): void {
    let wr = createWeakRefObject();
    let wrs = createWeakRefString('world!');
    let wrWithReachable = new WeakRef<Object>(reachableObj);
    // Run Full GC for referent collection
    try {
        let gcId = GC.startGC(GC.Cause.FULL);
        GC.waitForFinishGC(gcId);
    } catch (e) {
        throw new Error('Unexpected error during GC');
    }
    arktest.assertEQ(wr.deref(), undefined, 'Object referent must be collected after Full GC');
    arktest.assertEQ(wrs.deref(), undefined, 'String referent must be collected after Full GC');
    arktest.assertNE(reachableObj, null, 'Local object must not be collected');
    arktest.assertNE(wrWithReachable.deref(), undefined, 'deref for weak reference with reachable object ' +
    'must return non-undefined value');
}

function main(): int {
    fullGCWeakRefsMarkingCheck();

    return 0;
}
