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

class PinningTestsuite extends ArkTestsuite {
    constructor() {
        super("PinObject");
    }

    override before(): void {
        try {
            GC.waitForFinishGC(GC.startGC(GC.FULL_CAUSE)); // Clear heap
        } catch(e) {
            assertTrue(!"Unexpected exception during FULL GC");
        }
    }
}

function main(): int {
    let pinObjTestsuite = new PinningTestsuite();
    pinObjTestsuite.addTest("SimpleTest", (): void throws =>
    {
        let obj: Object = new Object();
        GC.pinObject(obj);
        assertEQ(GC.getObjectSpaceType(obj), GC.YOUNG_SPACE,
                 "The object before GC must be placed in young space");
        let addr_before_gc: long = GC.getObjectAddress(obj);
        // Run GC
        let gc_id = GC.startGC(GC.MIXED_CAUSE);
        GC.waitForFinishGC(gc_id);
        let addr_after_gc: long = GC.getObjectAddress(obj);
        assertEQ(GC.getObjectSpaceType(obj), GC.TENURED_SPACE,
                 "The pinned object after GC must be placed in tenured space");
        GC.unpinObject(obj);

        assertEQ(addr_before_gc, addr_after_gc, "Pinned object must has a fixed address");
    });
    pinObjTestsuite.addTest("AllocatePinnedArrayTest", (): void throws =>
    {
        let obj = GC.allocatePinnedIntArray(10);
        assertEQ(GC.getObjectSpaceType(obj), GC.TENURED_SPACE, "The pinned array after GC must be placed in tenured space");
        let addr_before_gc: long = GC.getObjectAddress(obj);
        // Run GC
        let gc_id = GC.startGC(GC.MIXED_CAUSE);
        GC.waitForFinishGC(gc_id);
        let addr_after_gc: long = GC.getObjectAddress(obj);
        GC.unpinObject(obj);
        assertEQ(addr_before_gc, addr_after_gc, "Pinned array must has a fixed address");
    });
    pinObjTestsuite.addTest("AllocateNegativeSizePinnedArrayTest", (): void throws =>
    {
        let expectedError = new NegativeArraySizeError("The value must be non negative");
        expectError((): void throws => { GC.allocatePinnedBooleanArray(-1) }, expectedError);
        expectError((): void throws => { GC.allocatePinnedByteArray(-1) }, expectedError);
        expectError((): void throws => { GC.allocatePinnedCharArray(-1) }, expectedError);
        expectError((): void throws => { GC.allocatePinnedShortArray(-1) }, expectedError);
        expectError((): void throws => { GC.allocatePinnedIntArray(-1) }, expectedError);
        expectError((): void throws => { GC.allocatePinnedLongArray(-1) }, expectedError);
        expectError((): void throws => { GC.allocatePinnedFloatArray(-1) }, expectedError);
        expectError((): void throws => { GC.allocatePinnedDoubleArray(-1) }, expectedError);
    });
    return pinObjTestsuite.run();
}
