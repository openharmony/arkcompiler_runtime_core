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

function simpleTest(): void throws {
    let obj: Object = new Object();
    assert obj != null : "Allocated object must not be null";
    GC.pinObject(obj);
    assert GC.getObjectSpaceType(obj) == GC.YOUNG_SPACE : "The object before GC must be placed in young space";
    let addr_before_gc: long = GC.getObjectAddress(obj);
    // Run GC
    let gc_id = GC.startGC(GC.MIXED_CAUSE);
    GC.waitForFinishGC(gc_id);
    let addr_after_gc: long = GC.getObjectAddress(obj);
    assert GC.getObjectSpaceType(obj) == GC.TENURED_SPACE : "The pinned object after GC must be placed in tenured space";
    GC.unpinObject(obj);

    assert addr_before_gc == addr_after_gc : "Pinned object must has a fixed address.\n" +
    "Address before GC = " + addr_before_gc + "\n" +
    "Address  after GC = " + addr_after_gc;
}

function allocatePinnedArrayTest(): void throws {
    let obj = GC.allocatePinnedIntArray(10);
    assert obj != null : "Allocated object must not be null";
    assert GC.getObjectSpaceType(obj) == GC.YOUNG_SPACE : "The array before GC must be placed in young space";
    let addr_before_gc: long = GC.getObjectAddress(obj);
    // Run GC
    let gc_id = GC.startGC(GC.MIXED_CAUSE);
    GC.waitForFinishGC(gc_id);
    let addr_after_gc: long = GC.getObjectAddress(obj);
    assert GC.getObjectSpaceType(obj) == GC.TENURED_SPACE : "The pinned array after GC must be placed in tenured space";
    GC.unpinObject(obj);

    assert addr_before_gc == addr_after_gc : "Pinned array must has a fixed address.\n" +
    "Address before GC = " + addr_before_gc + "\n" +
    "Address  after GC = " + addr_after_gc;
}

function nullObjectPinTest(): void {
    let detected = false;
    try {
        GC.pinObject(null);
    } catch(e: NullPointerException) {
        detected = true;
    }
    catch (e) {}
    assert detected == true: "GC.pinObject must throw NullPointerException for null object";
}

function nullObjectUnpinTest(): void {
    let detected = false;
    try {
        GC.unpinObject(null);
    } catch(e: NullPointerException) {
        detected = true;
    }
    catch (e) {}
    assert detected == true: "GC.unpinObject must throw NullPointerException for null object";
}

function allocateNegativeSizePinnedArrayTest(): void {
    let detected = 0;
    try {
        GC.allocatePinnedBooleanArray(-1);
    } catch(e: NegativeArraySizeException) {
        detected += 1;
    }
    catch (e) {}
    assert detected == 1: "Boolean array pinning must throw NegativeArraySizeException";
    try {
        GC.allocatePinnedByteArray(-1);
    } catch(e: NegativeArraySizeException) {
        detected += 1;
    }
    catch (e) {}
    assert detected == 2: "Byte array pinning must throw NegativeArraySizeException";
    try {
        GC.allocatePinnedCharArray(-1);
    } catch(e: NegativeArraySizeException) {
        detected += 1;
    }
    catch (e) {}
    assert detected == 3: "Char array pinning must throw NegativeArraySizeException";
    try {
        GC.allocatePinnedShortArray(-1);
    } catch(e: NegativeArraySizeException) {
        detected += 1;
    }
    catch (e) {}
    assert detected == 4: "Short array pinning must throw NegativeArraySizeException";
    try {
        GC.allocatePinnedIntArray(-1);
    } catch(e: NegativeArraySizeException) {
        detected += 1;
    }
    catch (e) {}
    assert detected == 5: "Int array pinning must throw NegativeArraySizeException";
    try {
        GC.allocatePinnedLongArray(-1);
    } catch(e: NegativeArraySizeException) {
        detected += 1;
    }
    catch (e) {}
    assert detected == 6: "Long array pinning must throw NegativeArraySizeException";
    try {
        GC.allocatePinnedFloatArray(-1);
    } catch(e: NegativeArraySizeException) {
        detected += 1;
    }
    catch (e) {}
    assert detected == 7: "Float array pinning must throw NegativeArraySizeException";
    try {
        GC.allocatePinnedDoubleArray(-1);
    } catch(e: NegativeArraySizeException) {
        detected += 1;
    }
    catch (e) {}
    assert detected == 8: "Double array pinning must throw NegativeArraySizeException";
}

function main(): int {
    try {
        simpleTest();
        GC.waitForFinishGC(GC.startGC(GC.FULL_CAUSE)); // Clear heap
        allocatePinnedArrayTest();
    }
    catch (e) {
        assert false: "Tests must not throw exceptions in this case";
    }
    // Checks for null object arguments
    nullObjectPinTest();
    nullObjectUnpinTest();
    // Negative array size test
    allocateNegativeSizePinnedArrayTest();
    return 0;
}
