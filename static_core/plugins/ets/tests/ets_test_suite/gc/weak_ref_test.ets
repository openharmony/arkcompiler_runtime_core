/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

function CreateWeakRefObject(): WeakRef<Object>
{
    let obj = new Object();
    return new WeakRef<Object>(obj);
}

function CreateWeakRefForConcurrent(): WeakRef<Object>
{
    let huge_array_size: int = 100000;
    let obj = new long[huge_array_size];
    return new WeakRef<Object>(obj);
}

function CreateWeakRefString(s: string): WeakRef<String>
{
    let obj = "Hello " + s;
    return new WeakRef<String>(obj);
}

function CreateWeakRefStringForConcurrent(): WeakRef<String>
{
    let obj = "Hello";
    for (let i = 0; i < 20; ++i) {
        obj += obj;
    }
    return new WeakRef<String>(obj);
}

// Make it global because optimizer is free to kill local variables
const reachable_obj = new Object();

function FullGCWeakRefsMarkingCheck(): void
{
    let wr = CreateWeakRefObject();
    let wrs = CreateWeakRefString("world!");
    let wr_with_reachable = new WeakRef<Object>(reachable_obj);
    // Run Full GC for referent collection
    try {
        let gc_id = GC.startGC(GC.Cause.FULL);
        GC.waitForFinishGC(gc_id);
    } catch (e) {
        throw new Error("Unexpected exception during GC");
    }
    assertEQ(wr.deref(), undefined, "Object referent must be collected after Full GC");
    assertEQ(wrs.deref(), undefined, "String referent must be collected after Full GC");
    assertNE(reachable_obj, null, "Local object must not be collected");
    assertNE(wr_with_reachable.deref(), undefined, "deref for weak reference with reachable object " +
    "must return non-undefined value");
}

function ConcurrentWeakRefsMarkingCheck(): void
{
    let wr = CreateWeakRefForConcurrent();
    let wrs = CreateWeakRefStringForConcurrent();
    let wr_with_reachable = new WeakRef<Object>(reachable_obj);
    // Register native allocation to trigger special GC (NATIVE_ALLOC_CAUSE)
    // for referent collection during concurrent marking
    try {
        GC.registerNativeAllocation(2); // 2 is enough to trigger GC in this test
    } catch (e) {
        throw new Error("Unexpected exception during GC");
    }
    assertEQ(wr.deref(), undefined, "Object referent must be collected after native GC cause");
    assertEQ(wrs.deref(), undefined, "String referent must be collected after native GC cause");
    assertNE(reachable_obj, null, "Local object must not be collected");
    assertNE(wr_with_reachable.deref(), undefined, "deref for weak reference with reachable object " +
    "must return non-undefined value");
}

function main(): int {
    FullGCWeakRefsMarkingCheck();
    ConcurrentWeakRefsMarkingCheck();

    return 0;
}
