/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

let removed_object_callbacks: boolean[] = new boolean[3];

function callback(object: Object): void {
    let string: String = object as String;
    if (string.equals("First object removed")) {
        removed_object_callbacks[0] = true;
    } else if (string.equals("Third object removed")) {
        removed_object_callbacks[2] = true;
    } else {
        // Must be unreachable
        assert false : "Calling unexpected callback: " + string;
    }
}

function finalizationQueueTest(queue: FinalizationQueue | null): void {
    let object1: Object | null = new Object();
    let object1_token: Object = new Object();
    let object1_callbackArg: String = new String("First object removed");
    let object2: Object | null = new Object();
    let object2_token: Object = new Object();
    let object2_callbackArg: String = new String("Second object removed, it is an error");
    let object3: Object | null = new Object();
    let object3_callbackArg: String = new String("Third object removed");
    let unregistered_token: Object = new Object();
    queue.register(object1!, object1_callbackArg as Object, object1_token);
    queue.register(object2!, object2_callbackArg as Object, object2_token);
    queue.register(object3!, object3_callbackArg as Object);

    let unused_queue: FinalizationQueue | null = new FinalizationQueue(callback);
    
    // Try to unregister unknown tokens:
    queue.unregister(unregistered_token);
    queue.unregister(null);
    unused_queue.unregister(object1);
    unused_queue.unregister(unregistered_token);
    unused_queue.unregister(null);

    // Check that we don't call any callbacks here
    try {
        let first_gc_id = GC.startGC(GC.FULL_CAUSE);
        GC.waitForFinishGC(first_gc_id);
    } catch (e) {
        assert false : "Unexpected exception during GC";
    }

    // Unregister object2, remove unused_queue
    // and check that we don't call object2 callback
    unused_queue = null;
    queue.unregister(object2_token);
    assert (object2.toString() as Object) != null : "Usage of object2 for preventing calling callbacks earlier";
    object2 = null;
    try {
        let second_gc_id = GC.startGC(GC.FULL_CAUSE);
        GC.waitForFinishGC(second_gc_id);
    } catch (e) {
        assert false : "Unexpected exception during GC";
    }

    // Clear objects and check correct callback calls:
    assert (object1.toString() as Object) != null : "Usage of object1 for preventing calling callbacks earlier";
    assert (object3.toString() as Object) != null : "Usage of object3 for preventing calling callbacks earlier";
    object1 = null;
    object3 = null;
    try {
        let third_gc_id = GC.startGC(GC.FULL_CAUSE);
        GC.waitForFinishGC(third_gc_id);
    } catch (e) {
        assert false : "Unexpected exception during GC";
    }
}

function main(): void {
    for (let i = 0; i < removed_object_callbacks.length; i++) {
        removed_object_callbacks[i] = false;
    }
    let queue: FinalizationQueue = new FinalizationQueue(callback);
    finalizationQueueTest(queue);
    // Check that we successfully remove FinalizationQueue instance
    // elemets and this cause no error during collection.
    try {
        let gc_id = GC.startGC(GC.FULL_CAUSE);
        GC.waitForFinishGC(gc_id);
    } catch (e) {
        assert false : "Unexpected exception during GC";
    }

    // Check that all callbacks have been called correctly.
    assert removed_object_callbacks[0] == true : "First callback hasn't been called";
    assert removed_object_callbacks[1] == false : "Second callback has been called";
    assert removed_object_callbacks[2] == true : "Third callback hasn't been called";
    assert queue != null : "Instance still alive";
}
