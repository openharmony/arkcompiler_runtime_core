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

let removed_object_callbacks: boolean[] = new boolean[3];
// Make it global beacuse optimizer is free to kill local variables
let object1 : Object | null = null;
let object2 : Object | null = null;
let object3 : Object | null = null;
let queue: FinalizationRegistry | null = null;
let unused_queue: FinalizationRegistry | null = null;

function doFullGC() {
    try {
        GC.waitForFinishGC(GC.startGC(GC.FULL_CAUSE));
    } catch (e) {
        assert false : "Unexpected exception during GC";
    }
}

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

function finalizationRegistryTest(queue: FinalizationRegistry | null): void {
    object1 = new Object();
    let object1_token = new Object();
    let object1_callbackArg = new String("First object removed");
    object2 = new Object();
    let object2_token = new Object();
    let object2_callbackArg = new String("Second object removed, it is an error");
    object3 = new Object();
    let object3_callbackArg = new String("Third object removed");
    let unregistered_token = new Object();
    queue!.register(object1!, object1_callbackArg as Object, object1_token);
    queue!.register(object2!, object2_callbackArg as Object, object2_token);
    queue!.register(object3!, object3_callbackArg as Object);

    unused_queue = new FinalizationRegistry(callback);

    // Try to unregister unknown tokens:
    queue!.unregister(unregistered_token);
    queue!.unregister(null);
    unused_queue!.unregister(object1);
    unused_queue!.unregister(unregistered_token);
    unused_queue!.unregister(null);

    // Check that we don't call any callbacks here
    doFullGC();

    // Unregister object2, remove unused_queue
    // and check that we don't call object2 callback
    unused_queue = null;
    queue!.unregister(object2_token);
    assert object2 != null : "Object 2 is alive";
    object2 = null;
    doFullGC();

    // Clear objects and check correct callback calls:
    assert object1 != null : "Object 1 is alive";
    assert object3 != null : "Object 3 is alive";
    object1 = null;
    object3 = null;
    doFullGC();
}

function main(): void {
    for (let i = 0; i < removed_object_callbacks.length; i++) {
        removed_object_callbacks[i] = false;
    }
    queue = new FinalizationRegistry(callback);
    finalizationRegistryTest(queue);
    // Check that we successfully remove FinalizationRegistry instance
    // elemets and this cause no error during collection.
    doFullGC();

    // Check that all callbacks have been called correctly.
    assert removed_object_callbacks[0] == true : "First callback hasn't been called";
    assert removed_object_callbacks[1] == false : "Second callback has been called";
    assert removed_object_callbacks[2] == true : "Third callback hasn't been called";
    assert queue != null : "Instance still alive";
}
