/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
package escompat;

export class Atomics {

    // https://tc39.es/ecma262/multipage/structured-data.html#sec-validateatomicaccess
    private static validateAtomicAccess(startByteOffset: int, elementSize: int, length: int, index: int): int {
        if (index < 0 || index >= length) {
            throw new RangeError("Index out of bounds")
        }
        return startByteOffset + (index * elementSize)
    }

    private static interpretWaitResult(waitResult: int): string {
        if (waitResult == 0) {
            return "ok"
        } else if (waitResult == 1) {
            return "not-equal"
        } else if (waitResult == 2) {
            return "timed-out"
        } else {
            throw new Error("unexpected WaitResult")
        }
    }

    private static requireSharedMemory(buffer: Buffer): SharedMemory {
        if (buffer instanceof ArrayBuffer) {
            throw new TypeError("This method accepts only TypedArrays that view SharedArrayBuffers")
        } else {
            let mem = (buffer as SharedArrayBuffer).getSharedMemory()
            return mem
        }
    }

    /**
     * isLockFree(n) returns true if Atomic operations for typed arrays where "BYTER_PER_ELEMENT == n"
     * use hardware atomics instructions instead of locks.
     *
     * Warning: currently, all Atomic operations use locks, but isLockFree(4) following the ECMA specification returns true.  
     */
    public isLockFree(byteSize: int): boolean {
        return byteSize == 4
    }

    // Int8Array

    public static add(typedArray: Int8Array, index: int, value: byte): byte {
        let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset, Int8Array.BYTES_PER_ELEMENT, typedArray.length, index)
        let buffer = typedArray.buffer
        if (buffer instanceof ArrayBuffer) {
            let oldValue = typedArray.at(indexedPosition)
            let newValue = (oldValue + value) as byte
            typedArray.set(indexedPosition, newValue)
            return oldValue
        } else {
            let mem = (buffer as SharedArrayBuffer).getSharedMemory()
            return mem.atomicAddI8(indexedPosition, value)
        }
    }

    public static and(typedArray: Int8Array, index: int, value: byte): byte {
        let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset, Int8Array.BYTES_PER_ELEMENT, typedArray.length, index)
        let buffer = typedArray.buffer
        if (buffer instanceof ArrayBuffer) {
            let oldValue = typedArray.at(indexedPosition)
            let newValue = (oldValue & value) as byte
            typedArray.set(indexedPosition, newValue)
            return oldValue
        } else {
            let mem = (buffer as SharedArrayBuffer).getSharedMemory()
            return mem.atomicAndI8(indexedPosition, value)
        }
    }

    public static compareExchange(typedArray: Int8Array, index: int, expectedValue: byte, replacementValue: byte): byte {
        let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset, Int8Array.BYTES_PER_ELEMENT, typedArray.length, index)
        let buffer = typedArray.buffer
        if (buffer instanceof ArrayBuffer) {
            let oldValue = typedArray.at(indexedPosition)
            if (oldValue == expectedValue) {
                typedArray.set(indexedPosition, replacementValue)
            }
            return oldValue
        } else {
            let mem = (buffer as SharedArrayBuffer).getSharedMemory()
            return mem.atomicCompareExchangeI8(indexedPosition, expectedValue, replacementValue)
        }
    }

    public static exchange(typedArray: Int8Array, index: int, value: byte): byte {
        let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset, Int8Array.BYTES_PER_ELEMENT, typedArray.length, index)
        let buffer = typedArray.buffer
        if (buffer instanceof ArrayBuffer) {
            let oldValue = typedArray.at(indexedPosition)
            typedArray.set(indexedPosition, value)
            return oldValue
        } else {
            let mem = (buffer as SharedArrayBuffer).getSharedMemory()
            return mem.atomicExchangeI8(indexedPosition, value)
        }
    }

    public static load(typedArray: Int8Array, index: int): byte {
        let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset, Int8Array.BYTES_PER_ELEMENT, typedArray.length, index)
        let buffer = typedArray.buffer
        if (buffer instanceof ArrayBuffer) {
            return typedArray.at(indexedPosition)
        } else {
            let mem = (buffer as SharedArrayBuffer).getSharedMemory()
            return mem.atomicLoadI8(indexedPosition)
        }
    }

    public static or(typedArray: Int8Array, index: int, value: byte): byte {
        let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset, Int8Array.BYTES_PER_ELEMENT, typedArray.length, index)
        let buffer = typedArray.buffer
        if (buffer instanceof ArrayBuffer) {
            let oldValue = typedArray.at(indexedPosition)
            let newValue = (oldValue + value) as byte
            typedArray.set(indexedPosition, newValue)
            return oldValue
        } else {
            let mem = (buffer as SharedArrayBuffer).getSharedMemory()
            return mem.atomicOrI8(indexedPosition, value)
        }
    }

    public static store(typedArray: Int8Array, index: int, value: byte): byte {
        let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset, Int8Array.BYTES_PER_ELEMENT, typedArray.length, index)
        let buffer = typedArray.buffer
        if (buffer instanceof ArrayBuffer) {
            typedArray.set(indexedPosition, value)
            return value
        } else {
            let mem = (buffer as SharedArrayBuffer).getSharedMemory()
            return mem.atomicStoreI8(indexedPosition, value)
        }
    }

    public static sub(typedArray: Int8Array, index: int, value: byte): byte {
        let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset, Int8Array.BYTES_PER_ELEMENT, typedArray.length, index)
        let buffer = typedArray.buffer
        if (buffer instanceof ArrayBuffer) {
            let oldValue = typedArray.at(indexedPosition)
            let newValue = (oldValue - value) as byte
            typedArray.set(indexedPosition, newValue)
            return oldValue
        } else {
            let mem = (buffer as SharedArrayBuffer).getSharedMemory()
            return mem.atomicSubI8(indexedPosition, value)
        }
    }

    public static xor(typedArray: Int8Array, index: int, value: byte): byte {
        let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset, Int8Array.BYTES_PER_ELEMENT, typedArray.length, index)
        let buffer = typedArray.buffer
        if (buffer instanceof ArrayBuffer) {
            let oldValue = typedArray.at(indexedPosition)
            let newValue = (oldValue ^ value) as byte
            typedArray.set(indexedPosition, newValue)
            return oldValue
        } else {
            let mem = (buffer as SharedArrayBuffer).getSharedMemory()
            return mem.atomicXorI8(indexedPosition, value)
        }
    }

    // Int16Array

    public static add(typedArray: Int16Array, index: int, value: short): short {
        throw new Error("not implemented")
    }

    public static and(typedArray: Int16Array, index: int, value: short): short {
        throw new Error("not implemented")
    }

    public static compareExchange(typedArray: Int16Array, index: int, expectedValue: short, replacementValue: short): short {
        throw new Error("not implemented")
    }

    public static exchange(typedArray: Int16Array, index: int, value: short): short {
        throw new Error("not implemented")
    }

    public static load(typedArray: Int16Array, index: int): short {
        throw new Error("not implemented")
    }

    public static or(typedArray: Int16Array, index: int, value: short): short {
        throw new Error("not implemented")
    }

    public static store(typedArray: Int16Array, index: int, value: short): short {
        throw new Error("not implemented")
    }

    public static sub(typedArray: Int16Array, index: int, value: short): short {
        throw new Error("not implemented")
    }

    public static xor(typedArray: Int16Array, index: int, value: short): short {
        throw new Error("not implemented")
    }

    // Int32Array

    public static add(typedArray: Int32Array, index: int, value: int): int {
        throw new Error("not implemented")
    }

    public static and(typedArray: Int32Array, index: int, value: int): int {
        throw new Error("not implemented")
    }

    public static compareExchange(typedArray: Int32Array, index: int, expectedValue: int, replacementValue: int): int {
        throw new Error("not implemented")
    }

    public static exchange(typedArray: Int32Array, index: int, value: int): int {
        throw new Error("not implemented")
    }

    public static load(typedArray: Int32Array, index: int): int {
        throw new Error("not implemented")
    }

    public static or(typedArray: Int32Array, index: int, value: int): int {
        throw new Error("not implemented")
    }

    public static store(typedArray: Int32Array, index: int, value: int): int {
        throw new Error("not implemented")
    }

    public static sub(typedArray: Int32Array, index: int, value: int): int {
        throw new Error("not implemented")
    }

    public static xor(typedArray: Int32Array, index: int, value: int): int {
        throw new Error("not implemented")
    }

    /**
     * If "typedArray[offset] != value" suspends the current thread until it is notified by Atomics.notify.
     *
     * Note: An Atomics.notify call will wake up this thread even if "typedArray[offset] == value".  
     */
    public static wait(typedArray: Int32Array, offset: int, value: int): string {
        let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset, Int32Array.BYTES_PER_ELEMENT, typedArray.length, offset)
        let mem = Atomics.requireSharedMemory(typedArray.buffer)
        let result = mem.atomicWaitI32(indexedPosition, value)
        return Atomics.interpretWaitResult(result)
    }

    /**
     * If "typedArray[offset] != value" suspends the current thread until it is notified by Atomics.notify
     * or until the given timeout passes.
     *
     * Note: An Atomics.notify call will wake up this thread even if "typedArray[offset] == value".
     */
    public static wait(typedArray: Int32Array, offset: int, value: int, timeout: long): string {
        let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset, Int32Array.BYTES_PER_ELEMENT, typedArray.length, offset)
        let mem = Atomics.requireSharedMemory(typedArray.buffer)
        let result = mem.atomicTimedWaitI64(indexedPosition, value, timeout)
        return Atomics.interpretWaitResult(result)
    }

    /**
     * Notifies (wakes up) threads that are suspended by the Atomics.wait() calls at the given index.
     * (index = typedArray.byteOffset + offset * 4)
     *
     * Note: This method also wakes up threads suspended by the BigInt64Array Atomics.wait(t64, offset64) calls.
     * But if and only if 't64' views the same Buffer as 'typedArray' and 'offset64' and 'offset' point at the same index in that Buffer.   
     */
    public static notify(typedArray: Int32Array, offset: int): int {
        let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset, Int32Array.BYTES_PER_ELEMENT, typedArray.length, offset)
        let buffer = typedArray.buffer
        if (buffer instanceof ArrayBuffer) {
            return 0
        } else {
            let mem = (buffer as SharedArrayBuffer).getSharedMemory()
            return mem.atomicNotify(indexedPosition)
        }
    }

    /**
     * Operates exactly like Atomics.notify(Int32Array, int) but specifies the maximum number of threads to notify using 'count'.  
     */
    public static notify(typedArray: Int32Array, offset: int, count: int): int {
        let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset, Int32Array.BYTES_PER_ELEMENT, typedArray.length, offset)
        let buffer = typedArray.buffer
        if (buffer instanceof ArrayBuffer) {
            return 0
        } else {
            let mem = (buffer as SharedArrayBuffer).getSharedMemory()
            return mem.atomicBoundedNotify(indexedPosition, count)
        }
    }

    // BigInt64Array

    public static add(typedArray: BigInt64Array, index: int, value: long): long {
        throw new Error("not implemented")
    }

    public static and(typedArray: BigInt64Array, index: int, value: long): long {
        throw new Error("not implemented")
    }

    public static compareExchange(typedArray: BigInt64Array, index: int, expectedValue: long, replacementValue: long): long {
        throw new Error("not implemented")
    }

    public static exchange(typedArray: BigInt64Array, index: int, value: long): long {
        throw new Error("not implemented")
    }

    public static load(typedArray: BigInt64Array, index: int): long {
        throw new Error("not implemented")
    }

    public static or(typedArray: BigInt64Array, index: int, value: long): long {
        throw new Error("not implemented")
    }

    public static store(typedArray: BigInt64Array, index: int, value: long): long {
        throw new Error("not implemented")
    }

    public static sub(typedArray: BigInt64Array, index: int, value: long): long {
        throw new Error("not implemented")
    }

    public static xor(typedArray: BigInt64Array, index: int, value: long): long {
        throw new Error("not implemented")
    }

    /**
     * If "typedArray[offset] != value" suspends the current thread until it is notified by Atomics.notify.
     *
     * Note: An Atomics.notify call will wake up this thread even if "typedArray[offset] == value".  
     */
    public static wait(typedArray: BigInt64Array, offset: int, value: long): string {
        let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset, Int32Array.BYTES_PER_ELEMENT, typedArray.length, offset)
        let mem = Atomics.requireSharedMemory(typedArray.buffer)
        let result = mem.atomicWaitI64(indexedPosition, value)
        return Atomics.interpretWaitResult(result)
    }

    /**
     * If "typedArray[offset] != value" suspends the current thread until it is notified by Atomics.notify
     * or until the given timeout passes.
     *
     * Note: An Atomics.notify call will wake up this thread even if "typedArray[offset] == value".
     */
    public static wait(typedArray: BigInt64Array, offset: int, value: long, timeout: long): string {
        let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset, Int32Array.BYTES_PER_ELEMENT, typedArray.length, offset)
        let mem = Atomics.requireSharedMemory(typedArray.buffer)
        let result = mem.atomicTimedWaitI64(indexedPosition, value, timeout)
        return Atomics.interpretWaitResult(result)
    }

    /**
     * Notifies (wakes up) threads that are suspended by the Atomics.wait() calls at the given index.
     * (index = typedArray.byteOffset + offset * 8)
     *
     * Note: This method also wakes up threads suspended by the Int32Array Atomics.wait(t32, offset32) calls.
     * But if and only if 't32' views the same Buffer as 'typedArray' and 'offset32' and 'offset' point at the same index in that Buffer.   
     */
    public static notify(typedArray: BigInt64Array, offset: int): long {
        let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset, BigInt64Array.BYTES_PER_ELEMENT, typedArray.length, offset)
        let buffer = typedArray.buffer
        if (buffer instanceof ArrayBuffer) {
            return 0
        } else {
            let mem = (buffer as SharedArrayBuffer).getSharedMemory()
            return mem.atomicNotify(indexedPosition)
        }
    }

    /**
     * Operates exactly like Atomics.notify(Int32Array, int) but specifies the maximum number of threads to notify using 'count'.  
     */
    public static notify(typedArray: BigInt64Array, offset: int, count: int): long {
        let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset, BigInt64Array.BYTES_PER_ELEMENT, typedArray.length, offset)
        let buffer = typedArray.buffer
        if (buffer instanceof ArrayBuffer) {
            return 0
        } else {
            let mem = (buffer as SharedArrayBuffer).getSharedMemory()
            return mem.atomicBoundedNotify(indexedPosition, count)
        }
    }
}
