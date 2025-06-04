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

function check_exceptions(): void {
    let exception_catched: boolean = false;
    try {
        GC.postponeGCEnd();
    } catch(error: IllegalStateException) {
        exception_catched = true;
    }
    assertTrue(exception_catched, "Didn't catch the exception");
    exception_catched = false;
    try {
        GC.postponeGCStart();
        GC.postponeGCStart();
    } catch(error: IllegalStateException) {
        exception_catched = true;
    }
    assertTrue(exception_catched, "Didn't catch the exception");
    GC.postponeGCEnd();
    exception_catched = false;
    try {
        GC.postponeGCStart();
        let gc_id = GC.startGC(GC.THRESHOLD_CAUSE);
        GC.waitForFinishGC(gc_id);
    } catch(error: IllegalStateException) {
        exception_catched = true;
    } catch (e) {
        assertTrue(false, "Unexpected exception during GC");
    }
    assertTrue(exception_catched, "Didn't catch the exception");
    GC.postponeGCEnd();
}


function main(): void {
    check_exceptions();
    let elements_count: int = 10000;
    let obj_holder: Object[] = new Object[elements_count];
    let obj_addr: long[] = new long[elements_count];

    GC.postponeGCStart();
    // Run GC for preventing a race between setting postpone GC start
    // and a collection in the GC thread
    try {
        let full_gc_id = GC.startGC(GC.FULL_CAUSE);
        GC.waitForFinishGC(full_gc_id);
    } catch (e) {
        assertTrue(false, "Unexpected exception during GC");
    }
    // Allocate elements
    for (let i: int = 0; i < elements_count; i++) {
        let new_object: Object = new Object();
        obj_holder[i] = new_object;
        obj_addr[i] = GC.getObjectAddress(new_object)
    }
    try {
        let young_gc_id = GC.startGC(GC.YOUNG_CAUSE);
        GC.waitForFinishGC(young_gc_id);
    } catch (e) {
        assertTrue(false, "Unexpected exception during GC");
    }
    // Check that we didn't move any object (i.e. didn't waste any time for that)
    for (let i: int = 0; i < elements_count; i++) {
        assertEQ(obj_addr[i], GC.getObjectAddress(obj_holder[i]), "The object has been moved");
    }
    // Allocate new object in young space
    let young_object: Object = new Object();
    let young_object_addr: long = GC.getObjectAddress(young_object);
    try {
        assertEQ(GC.YOUNG_SPACE, GC.getObjectSpaceType(young_object), "The object is not in young");
        GC.postponeGCEnd();
        // Check that the object will be moved after executing postpone end function.
        let gc_id = GC.startGC(GC.YOUNG_CAUSE);
        GC.waitForFinishGC(gc_id);
        assertNE(young_object_addr, GC.getObjectAddress(young_object), "The object hasn't been moved");
    } catch (e) {
        assertTrue(false, "Unexpected exception during GC");
    }
}
