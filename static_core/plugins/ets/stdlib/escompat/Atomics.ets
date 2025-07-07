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
package escompat;

import { launch } from "std/concurrency"

class Waiter {
    public static okStatus: string = "ok";
    public static notEqualStatus: string = "not-equal";
    public static timeOutStatus: string = "timed-out";

    public wait(val: long, getCurrentValue: () => long, timeout?: number): string {
        ConcurrencyHelpers.mutexLock(this.mutex);
        let check = getCurrentValue();
        console.log(check, val);
        if (val != check) {
            ConcurrencyHelpers.mutexUnlock(this.mutex);
            return Waiter.notEqualStatus;
        }
        if(!(timeout instanceof undefined) && (timeout as number) > 0) {
            setTimeout((waiter: Waiter) => {
                waiter.notify(Waiter.timeOutStatus);
            }, timeout as number, this);
        }
        let job = new CompletableJob<string>();
        this.promiseQueue.push(job);
        ConcurrencyHelpers.mutexUnlock(this.mutex);
        return job.Await();
    }

    public notify(val: string): boolean {
        ConcurrencyHelpers.mutexLock(this.mutex);
        let res = this.promiseQueue.poll();
        if(res instanceof null) {
            ConcurrencyHelpers.mutexUnlock(this.mutex);
            return false;
        }
        res!.finish(val);
        ConcurrencyHelpers.mutexUnlock(this.mutex);
        return true;
    }

    public notifyAll(val: string): int {
        let count = 0;
        ConcurrencyHelpers.mutexLock(this.mutex);
        for (let i: int = 0; i < this.promiseQueue.size; i++) {
            let res = this.promiseQueue.poll();
            if(res == null) {
                break;
            }
            res!.finish(val);
            count++;
        }
        ConcurrencyHelpers.mutexUnlock(this.mutex);
        return count;
    }


    private mutex: Object = ConcurrencyHelpers.mutexCreate();
    private promiseQueue: BlockingQueue<CompletableJob<string>> = new LinkedBlockingQueue<CompletableJob<string>>();
}

export class Atomics {

    /**
     * isLockFree(n) returns true if Atomic operations for typed arrays where "BYTER_PER_ELEMENT == n"
     * use hardware atomics instructions instead of locks.
     *
     * Warning: currently, all Atomic operations use locks,
     * but isLockFree(1), isLockFree(2), isLockFree(4), isLockFree(8) following the ECMA specification return true.
     */
    public static isLockFree(byteSize: int): boolean {
        return byteSize == 1 || byteSize == 2 || byteSize == 4 || byteSize == 8
    }

    /**
     * Atomically adds value to current value and replaces it with the result
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @param { number } value value to add to current value
     * @returns { number } current value before the operation
     */
    public static add(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: number, value: number): number {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof Int8Array) {
            return buffer.atomicAddI8(iindex, byteOffset, value as byte)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicAddI16(iindex, byteOffset, value as short)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicAddI32(iindex, byteOffset, value as int)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicAddU8(iindex, byteOffset, value as byte)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicAddU16(iindex, byteOffset, value as short)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicAddU32(iindex, byteOffset, value as int)
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * Atomically adds value to current value and replaces it with the result
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @param { bigint } value value to add to current value
     * @returns { bigint } current value before the operation
     */
    public static add(typedArray: BigInt64Array | BigUint64Array, index: number, value: bigint): bigint {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof BigUint64Array) {
            return new BigInt(buffer.atomicAddU64(iindex, byteOffset, value.getULong()))
        } else if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicAddI64(iindex, byteOffset, value.getLong()))
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * Atomically replaces current value it with the result of bitwise AND with value
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @param { number } value value to perform bitwise AND with current value
     * @returns { number } current value before the operation
     */
    public static and(typedArray: Int8Array | Int16Array | Int32Array| Uint8Array | Uint16Array | Uint32Array, index: number, value: number): number {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof Int8Array) {
            return buffer.atomicAndI8(iindex, byteOffset, value as byte)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicAndI16(iindex, byteOffset, value as short)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicAndI32(iindex, byteOffset, value as int)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicAndU8(iindex, byteOffset, value as byte)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicAndU16(iindex, byteOffset, value as short)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicAndU32(iindex, byteOffset, value as int)
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * Atomically replaces current value it with the result of bitwise AND with value
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @param { bigint } value value to perform bitwise AND with current value
     * @returns { bigint } current value before the operation
     */
    public static and(typedArray: BigInt64Array | BigUint64Array, index: number, value: bigint): bigint {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof BigUint64Array) {
            return new BigInt(buffer.atomicAndI64(iindex, byteOffset, value.getULong()))
        } else if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicAndU64(iindex, byteOffset, value.getLong()))
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * Performs a compare and swap operation with the location from the typed array
     * Atomically compares current value with the expected value. It they're equal, replaces
     * current value with the new value, else does nothing
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @param { number } expectedValue expected value for the current value
     * @param { number } replacementValue value to replace current value with in case of success
     * @returns { number } current value before the operation
     */
    public static compareExchange(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: number, expectedValue: number, replacementValue: number): number {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof Int8Array) {
            return buffer.atomicCompareExchangeI8(iindex, byteOffset, expectedValue as byte, replacementValue as byte)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicCompareExchangeI16(iindex, byteOffset, expectedValue as short, replacementValue as short)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicCompareExchangeI32(iindex, byteOffset, expectedValue as int, replacementValue as int)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicCompareExchangeU8(iindex, byteOffset, expectedValue as byte, replacementValue as byte)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicCompareExchangeU16(iindex, byteOffset, expectedValue as short, replacementValue as short)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicCompareExchangeU32(iindex, byteOffset, expectedValue as int, replacementValue as int)
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * Performs a compare and swap operation with the location from the typed array
     * Atomically compares current value with the expected value
     * If they're equal, replaces current value with the new value
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @param { bigint } expectedValue expected value for the current value
     * @param { bigint } replacementValue value to replace current value with in case of success
     * @returns { bigint } current value before the operation
     */
    public static compareExchange(typedArray: BigInt64Array | BigUint64Array, index: number, expectedValue: bigint, replacementValue: bigint): bigint {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof BigUint64Array) {
            return new BigInt(buffer.atomicCompareExchangeI64(iindex, byteOffset, expectedValue.getULong(), replacementValue.getULong()))
        } else if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicCompareExchangeU64(iindex, byteOffset, expectedValue.getLong(), replacementValue.getLong()))
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * Performs an exchange operation with the location from the typed array
     * Atomically compares current value with the expected value
     * If they're equal, replaces current value with the new value
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @param { number } value value to replace current value with
     * @returns { number } current value before the operation
     */
    public static exchange(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: number, value: number): number {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof Int8Array) {
            return buffer.atomicExchangeI8(iindex, byteOffset, value as byte)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicExchangeI16(iindex, byteOffset, value as short)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicExchangeI32(iindex, byteOffset, value as int)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicExchangeU8(iindex, byteOffset, value as byte)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicExchangeU16(iindex, byteOffset, value as short)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicExchangeU32(iindex, byteOffset, value as int)
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * Performs an exchange operation with the location from the typed array
     * Atomically replaces current value with the new value
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @param { bigint } value value to replace current value with
     * @returns { bigint } current value before the operation
     */
    public static exchange(typedArray: BigInt64Array | BigUint64Array, index: number, value: bigint): bigint {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicExchangeI64(iindex, byteOffset, value.getLong()))
        } else if (typedArray instanceof BigUint64Array) {
            return new BigInt(buffer.atomicExchangeU64(iindex, byteOffset, value.getULong()))
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * Atomically reads value from the typed array
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @returns { number } current value
     */
    public static load(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: number): number {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof Int8Array) {
            return buffer.atomicLoadI8(iindex, byteOffset)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicLoadI16(iindex, byteOffset)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicLoadI32(iindex, byteOffset)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicLoadU8(iindex, byteOffset)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicLoadU16(iindex, byteOffset)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicLoadU32(iindex, byteOffset)
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * Atomically reads value from the typed array
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @returns { bigint } current value
     */
    public static load(typedArray: BigInt64Array, index: number): bigint {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        return new BigInt(buffer.atomicLoadI64(iindex, byteOffset))
    }

    /**
     * Atomically reads value from the typed array
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @returns { bigint } current value
     */
    public static load(typedArray: BigUint64Array, index: number): bigint {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        return new BigInt(buffer.atomicLoadU64(iindex, byteOffset))
    }

    /**
     * Atomically replaces current value it with the result of bitwise OR with value
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @param { number } value value to perform bitwise OR with current value
     * @returns { number } current value before the operation
     */
    public static or(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: number, value: number): number {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof Int8Array) {
            return buffer.atomicOrI8(iindex, byteOffset, value as byte)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicOrI16(iindex, byteOffset, value as short)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicOrI32(iindex, byteOffset, value as int)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicOrU8(iindex, byteOffset, value as byte)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicOrU16(iindex, byteOffset, value as short)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicOrU32(iindex, byteOffset, value as int)
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * Atomically replaces current value it with the result of bitwise OR with value
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @param { bigint } value value to perform bitwise OR with current value
     * @returns { bigint } current value before the operation
     */
    public static or(typedArray: BigInt64Array | BigUint64Array, index: number, value: bigint): bigint {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicOrI64(iindex, byteOffset, value.getLong()))
        } else if (typedArray instanceof BigUint64Array) {
            return new BigInt(buffer.atomicOrU64(iindex, byteOffset, value.getULong()))
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * Atomically writes new value into the typed array
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @param { number } value new value
     * @returns { number } new value
     */
    public static store(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: number, value: number): number {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof Int8Array) {
            return buffer.atomicStoreI8(iindex, byteOffset, value as byte)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicStoreI16(iindex, byteOffset, value as short)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicStoreI32(iindex, byteOffset, value as int)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicStoreU8(iindex, byteOffset, value as byte)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicStoreU16(iindex, byteOffset, value as short)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicStoreU32(iindex, byteOffset, value as int)
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * Atomically writes new value into the typed array
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @param { bigint } value new value
     * @returns { bigint } new value
     */
    public static store(typedArray: BigInt64Array | BigUint64Array, index: number, value: bigint): bigint {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicStoreI64(iindex, byteOffset, value.getLong()))
        } else if (typedArray instanceof BigUint64Array) {
            return new BigInt(buffer.atomicStoreU64(iindex, byteOffset, value.getULong()))
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * Atomically subtracts value from current value and replaces it with the result
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @param { number } value value to subtract from current value
     * @returns { number } current value before the operation
     */
    public static sub(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: number, value: number): number {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof Int8Array) {
            return buffer.atomicSubI8(iindex, byteOffset, value as byte)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicSubI16(iindex, byteOffset, value as short)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicSubI32(iindex, byteOffset, value as int)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicSubU8(iindex, byteOffset, value as byte)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicSubU16(iindex, byteOffset, value as short)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicSubU32(iindex, byteOffset, value as int)
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * Atomically subtracts value from current value and replaces it with the result
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @param { bigint } value value to subtract from current value
     * @returns { bigint } current value before the operation
     */
    public static sub(typedArray: BigInt64Array | BigUint64Array, index: number, value: bigint): bigint {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicSubI64(iindex, byteOffset, value.getLong()))
        } else if (typedArray instanceof BigUint64Array) {
            return new BigInt(buffer.atomicSubU64(iindex, byteOffset, value.getULong()))
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * Atomically replaces current value it with the result of bitwise XOR with value
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @param { number } value value to perform bitwise XOR with current value
     * @returns { number } current value before the operation
     */
    public static xor(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: number, value: number): number {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof Int8Array) {
            return buffer.atomicXorI8(iindex, byteOffset, value as byte)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicXorI16(iindex, byteOffset, value as short)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicXorI32(iindex, byteOffset, value as int)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicXorU8(iindex, byteOffset, value as byte)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicXorU16(iindex, byteOffset, value as short)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicXorU32(iindex, byteOffset, value as int)
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * Atomically replaces current value it with the result of bitwise XOR with value
     *
     * @param typedArray typed array of values
     * @param  { number } index index of current value in the typedArray
     * @param { bigint } value value to perform bitwise XOR with current value
     * @returns { bigint } current value before the operation
     */
    public static xor(typedArray: BigInt64Array | BigUint64Array, index: number, value: bigint): bigint {
        let buffer = typedArray.buffer
        let iindex = index as int;
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicXorI64(iindex, byteOffset, value.getLong()))
        } else if (typedArray instanceof BigUint64Array) {
            return new BigInt(buffer.atomicXorU64(iindex, byteOffset, value.getULong()))
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * If "typedArray[offset] != value" suspends the current thread until it is notified by Atomics.notify.
     *
     * Note: An Atomics.notify call will wake up this thread even if "typedArray[offset] == value".
     */
    public static wait(typedArray: Int32Array, index: number, value: number): string {
        return Atomics.waitImpl(typedArray, index, value as long);
    }

    public static wait(typedArray: Int32Array, index: number, value: number, timeout: number): string {
        return Atomics.waitImpl(typedArray, index, value as long, timeout);
    }

    public static wait(typedArray: BigInt64Array, index: number, value: bigint): string {
        return Atomics.waitImpl(typedArray, index, value.getLong());
    }

    public static wait(typedArray: BigInt64Array, index: number, value: bigint, timeout: number): string {
        return Atomics.waitImpl(typedArray, index, value.getLong(), timeout);
    }

    /**
     * If "typedArray[offset] != value" suspends the current thread until it is notified by Atomics.notify
     * or until the given timeout passes.
     *
     * Note: An Atomics.notify call will wake up this thread even if "typedArray[offset] == value".
     */
    public async static waitAsync(typedArray: Int32Array | BigInt64Array, index: number, value: bigint): Promise<string> {
        return Atomics.asyncWaitImpl(typedArray, index, value.getLong()) as Promise<string>;
    }

    public async static waitAsync(typedArray: Int32Array | BigInt64Array, index: number, value: bigint, timeout: number): Promise<string> {
        return Atomics.asyncWaitImpl(typedArray, index, value.getLong(), timeout) as Promise<string>;
    }

    /**
     * Notifies (wakes up) threads that are suspended by the Atomics.wait() calls at the given index.
     * (index = typedArray.byteOffset + offset * sizeof(arrayElement))
     */
    public static notify(typedArray: Int32Array | BigInt64Array, index: number): number {
        return Atomics.notifyImpl(typedArray, index)
    }

    /**
     * Operates exactly like Atomics.notify(Int32Array | BigInt64Array, int) but specifies the maximum number of threads to notify using 'count'.
     */
    public static notify(typedArray: Int32Array | BigInt64Array, index: number, count: number): number {
        return Atomics.notifyImpl(typedArray, index, count)
    }

    public static waitImpl(typedArray: Int32Array | BigInt64Array, index: number, value: long, timeout?: number): string {
        // Getting correct waiter
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof Int32Array) {
            let offset = Atomics.index2byteOffset(byteOffset, Int32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let waiter = Atomics.getWaiter(buffer, offset);
            return waiter!.wait(value, () => { return buffer.atomicLoadI32(index as int, typedArray.byteOffset as int) as long; }, timeout)
        } else {
            let offset = Atomics.index2byteOffset(byteOffset, BigInt64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let waiter = Atomics.getWaiter(buffer, offset);
            return waiter!.wait(value, () => { return buffer.atomicLoadI64(index.toInt(), typedArray.byteOffset.toInt()).toLong(); }, timeout)
        }
    }
    
    private static asyncWaitImpl(typedArray: Int32Array | BigInt64Array, index: number, value: long, timeout?: number): Promise<string> {
        let resolver: (((v: String) => void) | undefined) = undefined;
        let rejecter: (((error: Error) => void) | undefined) = undefined;
        let p = new Promise<String>((res, rej) => {
            resolver = res
            rejecter = rej;
        })
        let cb = () => {
            try {
                let v = Atomics.waitImpl(typedArray, index, value, number)
                resolver!(v)
            } catch (e: Error) {
                rejecter!(e)
            }
        }
        launch<void, () => void>(cb)
        return p;
    }

    private static notifyImpl(typedArray: Int32Array | BigInt64Array, index: number, count?: number): int {
        let buffer = typedArray.buffer
        let offset = 0;
        let byteOffset = typedArray.byteOffset as int;
        if (typedArray instanceof Int32Array) {
            offset = Atomics.index2byteOffset(byteOffset, Int32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
        } else {
            offset = Atomics.index2byteOffset(byteOffset, BigInt64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
        }
        let waiter = Atomics.getWaiter(buffer, offset);
        let notified = 0;
        if (count == undefined) {
            notified = waiter!.notifyAll(Waiter.okStatus);
        } else {
            for(let i = 0; i < count; i++) {
                if (waiter!.notify(Waiter.okStatus)) {
                    notified++;
                }
            }
        }
        return notified;
    }

    private static globalWaitingTable: ConcurrentHashMap<ArrayBuffer, ConcurrentHashMap<int, Waiter>> = new ConcurrentHashMap<ArrayBuffer, ConcurrentHashMap<int, Waiter>>();
    private static globalWaitingTableMutex = ConcurrencyHelpers.mutexCreate();

    private static getWaiter(buffer: ArrayBuffer, byteOffset: int): Waiter {
        let waiterTable = Atomics.globalWaitingTable.get(buffer);
        if (waiterTable instanceof undefined) {
            ConcurrencyHelpers.mutexLock(Atomics.globalWaitingTableMutex)
            waiterTable = Atomics.globalWaitingTable.get(buffer)
            if (waiterTable instanceof undefined) {
                waiterTable = new ConcurrentHashMap<int, Waiter>()
                Atomics.globalWaitingTable.set(buffer, waiterTable!)
            }
            ConcurrencyHelpers.mutexUnlock(Atomics.globalWaitingTableMutex)
        }

        let waiter = waiterTable!.get(byteOffset)
        if (waiter instanceof undefined) {
            ConcurrencyHelpers.mutexLock(Atomics.globalWaitingTableMutex)
            waiter = waiterTable!.get(byteOffset)
            if (waiter instanceof undefined) {
                waiter = new Waiter()
                waiterTable!.set(byteOffset, waiter!)
            }
            ConcurrencyHelpers.mutexUnlock(Atomics.globalWaitingTableMutex)
        }
        return waiter as Waiter;
    }

    // https://tc39.es/ecma262/multipage/structured-data.html#sec-validateatomicaccess
    private static index2byteOffset(startByteOffset: int, elementSize: int, length: int, index: number): int {
        if (index < 0 || index >= length) {
            throw new RangeError("Index out of bounds")
        }
        return startByteOffset + (index as int * elementSize)
    }

}
