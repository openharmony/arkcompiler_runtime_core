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

let removed_object_callbacks: boolean[] = new boolean[4];
// Make it global beacuse optimizer is free to kill local variables
let object1 : Object | null = null;
let object2 : Object | null = null;
let object3 : Object | null = null;
let registry: FinalizationRegistry<String> | null = null;
let unusedRegistry: FinalizationRegistry<String> | null = null;
let emptyArgCallbackRegistry: FinalizationRegistry<Int> | null = null;

function doFullGC() {
    try {
        GC.waitForFinishGC(GC.startGC(GC.FULL_CAUSE));
    } catch (e) {
        assert false : "Unexpected exception during GC";
    }
}

function callback(string: String): void {
    if (string.equals("First object removed")) {
        removed_object_callbacks[0] = true;
    } else if (string.equals("Third object removed")) {
        removed_object_callbacks[2] = true;
    } else {
        // Must be unreachable
        assert false : "Calling unexpected callback: " + string;
    }
}

function emptyArgCallback(): void {
    removed_object_callbacks[3] = true;
}

function finalizationRegistryTest(registry: FinalizationRegistry<String> | null): void {
    object1 = new Object();
    let object1_token = new Object();
    let object1_callbackArg = new String("First object removed");
    object2 = new Object();
    let object2_token = new Object();
    let object2_callbackArg = new String("Second object removed, it is an error");
    object3 = new Object();
    let object3_callbackArg = new String("Third object removed");
    let unregistered_token = new Object();
    registry!.register(object1!, object1_callbackArg, object1_token);
    registry!.register(object2!, object2_callbackArg, object2_token);
    registry!.register(object3!, object3_callbackArg);

    unusedRegistry = new FinalizationRegistry<String>(callback);
    emptyArgCallbackRegistry = new FinalizationRegistry<Int>(emptyArgCallback);
    emptyArgCallbackRegistry!.register(object1!, 2, object1_token);

    // Try to unregister unknown tokens:
    registry!.unregister(unregistered_token);
    unusedRegistry!.unregister(object1!);
    unusedRegistry!.unregister(unregistered_token);

    // Check that we don't call any callbacks here
    doFullGC();

    // Unregister object2, remove unusedRegistry
    // and check that we don't call object2 callback
    unusedRegistry = null;
    registry!.unregister(object2_token);
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
    registry = new FinalizationRegistry<String>(callback);
    finalizationRegistryTest(registry);
    // Check that we successfully remove FinalizationRegistry instance
    // elemets and this cause no error during collection.
    doFullGC();

    // Check that all callbacks have been called correctly.
    assert removed_object_callbacks[0] == true : "First callback hasn't been called";
    assert removed_object_callbacks[1] == false : "Second callback has been called";
    assert removed_object_callbacks[2] == true : "Third callback hasn't been called";
    assert removed_object_callbacks[3] == true : "Fouth callback hasn't been called";
    assert registry != null : "Instance still alive";
}
