/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
    assert obj != null : "New object must be correctly allocated";
    return new WeakRef<Object>(obj);
}

function CreateWeakRefString(s: string): WeakRef<String>
{
    let obj = "Hello " + s;
    assert obj != null : "New string object must be correctly allocated";
    return new WeakRef<String>(obj);
}

// Make it global beacuse optimizer is free to kill local variables
const reachable_obj = new Object();

function main(): int {
    let wr = CreateWeakRefObject();
    let wrs = CreateWeakRefString("world!");
    let wr_with_reachable = new WeakRef<Object>(reachable_obj);
    // Run Full GC for referent collection
    try {
        let gc_id = GC.startGC(GC.FULL_CAUSE);
        GC.waitForFinishGC(gc_id);
    } catch (e) {
        assert false : "Unexpected exception during GC";
    }
    assert wr.deref() === undefined : "Object referent must be collected after Full GC";
    assert wrs.deref() === undefined : "String referent must be collected after Full GC";
    assert reachable_obj != null : "Local object must not be collected"
    assert wr_with_reachable.deref() != undefined : "deref for weak reference with reachable object " +
    "must return non-undefined value";
    return 0;
}
