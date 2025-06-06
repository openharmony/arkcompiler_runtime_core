/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

package std.core;

/**
 * @class GC defines methods with GC-specific behavior
 */
export final class GC {
    /**
     * @see getObjectSpaceType
     */
    public static readonly UNDEFINED_SPACE = 0;
    public static readonly OBJECT_SPACE = 1;
    public static readonly HUMONGOUS_SPACE = 2;
    public static readonly NON_MOVABLE_SPACE = 3;
    public static readonly YOUNG_SPACE = 4;
    public static readonly TENURED_SPACE = 5;

    /**
     * @see startGC
     * @see scheduleGcAfterNthAlloc
     */
    public static readonly YOUNG_CAUSE = 0;
    public static readonly THRESHOLD_CAUSE = 1;
    public static readonly MIXED_CAUSE = 2;
    public static readonly FULL_CAUSE = 3;

    public static readonly NOT_IN_PLACE_MODE: boolean = false;
    public static readonly IN_PLACE_MODE: boolean = true;

    // Use class as static
    private constructor() {}

    /**
     * @returns long - free heap space in bytes available for new objects
     */
    public static native getFreeHeapSize(): long;

    /**
     * @returns long - currently used heap space in bytes
     */
    public static native getUsedHeapSize(): long;

    /**
     * @returns long - maximum heap space size in bytes
     */
    public static native getReservedHeapSize(): long;

    /**
     * The function triggers specific GC
     *
     * @param cause cause of requested GC
     * @param callback callback is called in concurrent phase
     * @param inPlaceMode run GC in place mode
     *
     * @throws IllegalArgumentException if cause of GC is invalid or
     *         current GC does not support requested cause
     *
     * @returns id of started GC.
     * - The id can be passed to waitForFinishGC to ensure the GC is finished.
     * - 0 in case the GC is executed in-place. It means there is no need to wait such GC
     * - -1 in case the task is canceled
     */
    public static startGC(cause: int, callback: () => void, inPlaceMode: boolean = GC.NOT_IN_PLACE_MODE): long throws {
        return GC.startGCImpl(cause, callback as Object, inPlaceMode);
    }

    /**
     * The function triggers specific GC
     *
     * @param cause cause of requested GC
     * @param inPlaceMode run GC in place mode
     *
     * @throws IllegalArgumentException if cause of GC is invalid or
     *         current GC does not support requested cause
     * @throws UnsupportedOperationException if current GC does not support requested cause
     * @throws IllegalStateException if try to run threshold GC collection not in place after calling postponeGCStart
     *
     * @returns id of started GC.
     * - The id can be passed to waitForFinishGC to ensure the GC is finished.
     * - 0 in case the GC is executed in-place. It means there is no need to wait such GC
     * - -1 in case the task is canceled.
     */
    public static startGC(cause: int, inPlaceMode: boolean = GC.NOT_IN_PLACE_MODE): long throws {
        return GC.startGCImpl(cause, undefined, inPlaceMode);
    }
    
    private static native startGCImpl(cause: int, callback: Object | undefined, runInPlace: boolean): long throws;

    /**
    * The function returns when the specified GC gets finished
    *
    * @param gc_id id of the GC which is returned by startGC.
    *                      If gc_id is 0 or -1 the function returns immediately.
    */
    public static native waitForFinishGC(gc_id: long): void;

    /**
     * Method schedules GC before n-th allocation by setting counter to the specific GC trigger
     * Another call may reset the counter. In this case the last counter will be used to trigger the GC.
     *
     * @param counter number of allocation for trigger GC
     * @param cause cause of requested GC
     *
     * @throws IllegalArgumentException if cause of GC is invalid or
     *         current GC does not support requested cause
     * @throws UnsupportedOperationException if VM is running with incorrect GC trigger
     */
    public static native scheduleGcAfterNthAlloc(counter: int, cause: int): void throws;

    /**
     * Checks if GC was triggered
     *
     * @returns true if scheduled GC was triggered for a n-th allocation, false - otherwise
     *
     * @see scheduleGcAfterNthAlloc
     */
    public static native isScheduledGCTriggered(): boolean;

    /**
     * Start postponing GC collection - postpone GC for some time.
     * Disable threshold collections, young collections will promote everything.
     * Require to run postponeGCEnd after finishing critical code.
     *
     * @see postponeGCEnd
     *
     * @throws UnsupportedOperationException if GC doesn't support postponing
     * @throws IllegalStateException if it called after calling postponeGCStart
     *         without calling postponeGCEnd
     */
    public static native postponeGCStart(): void;

    /**
     * End postponing GC collection - finish postpone GC collections.
     * Should be called after postponeGCStart
     *
     * @see postponeGCStart
     *
     * @throws UnsupportedOperationException if GC doesn't support postponing
     * @throws IllegalStateException if it called without calling postponeGCStart
     */
    public static native postponeGCEnd(): void;

    /**
     * Pin object - GC will not move pinned object
     *
     * @param obj non-pinned object for pinning
     *
     * @see unpinObject
     *
     * @throws UnsupportedOperationException if used GC does not support pinning
     */
    public static native pinObject(obj: Object): void throws;

    /**
     * Unpin pinned object - now GC can move such object if it need
     *
     * @note Unpinning of non-pinned object is unspecified behavior
     *
     * @see pinObject
     * @see allocatePinned<PrimitiveType>Array
     *
     * @param obj pinned object for unpinning
     */
    public static native unpinObject(obj: Object): void;

    /**
     * Allocate array of boolean and pin (GC will not move pinned array)
     *
     * @param length count of array elements
     *
     * @see unpinObject
     *
     * @throws NegativeArraySizeError if length is negative
     * @throws UnsupportedOperationException if used GC does not support pinning
     * @throws OutOfMemoryError if VM has not enough heap space for array allocation
     *
     * @returns pinned array of boolean
     */
    public static native allocatePinnedBooleanArray(length: long): boolean[] throws;

    /**
     * Allocate array of byte and pin (GC will not move pinned array)
     *
     * @param length count of array elements
     *
     * @see unpinObject
     *
     * @throws NegativeArraySizeError if length is negative
     * @throws UnsupportedOperationException if used GC does not support pinning
     * @throws OutOfMemoryError if VM has not enough heap space for array allocation
     *
     * @returns pinned array of byte
     */
    public static native allocatePinnedByteArray(length: long): byte[] throws;

    /**
     * Allocate array of char and pin (GC will not move pinned array)
     *
     * @param length count of array elements
     *
     * @see unpinObject
     *
     * @throws NegativeArraySizeError if length is negative
     * @throws UnsupportedOperationException if used GC does not support pinning
     * @throws OutOfMemoryError if VM has not enough heap space for array allocation
     *
     * @returns pinned array of char
     */
    public static native allocatePinnedCharArray(length: long): char[] throws;

    /**
     * Allocate array of short and pin (GC will not move pinned array)
     *
     * @param length count of array elements
     *
     * @see unpinObject
     *
     * @throws NegativeArraySizeError if length is negative
     * @throws UnsupportedOperationException if used GC does not support pinning
     * @throws OutOfMemoryError if VM has not enough heap space for array allocation
     *
     * @returns pinned array of short
     */
    public static native allocatePinnedShortArray(length: long): short[] throws;

    /**
     * Allocate array of int and pin (GC will not move pinned array)
     *
     * @param length count of array elements
     *
     * @see unpinObject
     *
     * @throws NegativeArraySizeError if length is negative
     * @throws UnsupportedOperationException if used GC does not support pinning
     * @throws OutOfMemoryError if VM has not enough heap space for array allocation
     *
     * @returns pinned array of int
     */
    public static native allocatePinnedIntArray(length: long): int[] throws;

    /**
     * Allocate array of long and pin (GC will not move pinned array)
     *
     * @param length count of array elements
     *
     * @see unpinObject
     *
     * @throws NegativeArraySizeError if length is negative
     * @throws UnsupportedOperationException if used GC does not support pinning
     * @throws OutOfMemoryError if VM has not enough heap space for array allocation
     *
     * @returns pinned array of long
     */
    public static native allocatePinnedLongArray(length: long): long[] throws;

    /**
     * Allocate array of float and pin (GC will not move pinned array)
     *
     * @param length count of array elements
     *
     * @see unpinObject
     *
     * @throws NegativeArraySizeError if length is negative
     * @throws UnsupportedOperationException if used GC does not support pinning
     * @throws OutOfMemoryError if VM has not enough heap space for array allocation
     *
     * @returns pinned array of float
     */
    public static native allocatePinnedFloatArray(length: long): float[] throws;

    /**
     * Allocate array of double and pin (GC will not move pinned array)
     *
     * @param length count of array elements
     *
     * @see unpinObject
     *
     * @throws NegativeArraySizeError if length is negative
     * @throws UnsupportedOperationException if used GC does not support pinning
     * @throws OutOfMemoryError if VM has not enough heap space for array allocation
     *
     * @returns pinned array of double
     */
    public static native allocatePinnedDoubleArray(length: long): double[] throws;

    /**
     * Returns encoded space type
     *
     * @param obj object for which space type is requested
     *
     * @returns type of space which contains object-argument
     */
    public static native getObjectSpaceType(obj: Object): int;

    /**
     * Returns object address
     *
     * @param obj object for which address is requested
     *
     * @returns address of object
     */
    public static native getObjectAddress(obj: Object): long;

    /**
     * @param obj object for which size is requested
     * @returns size of the object in bytes
     */
    public static native getObjectSize(obj: Object): long;

    /**
     * @throws NegativeArraySizeError if size is negative
     *
     * @param size: long - number of bytes to be registered by GC as native
     */
    public static native registerNativeAllocation(size: long): void;

    /**
     * @throws NegativeArraySizeError if size is negative
     *
     * @param size: long - number of bytes to be freed by GC
     */
    public static native registerNativeFree(size: long): void;
}
