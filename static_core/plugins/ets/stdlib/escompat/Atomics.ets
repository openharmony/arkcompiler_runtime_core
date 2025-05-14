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

type Resolver = (value: string) => void;

class Waiter {
    public static okStatus: string = "ok";
    public static notEqualStatus: string = "not-equal";
    public static timeOutStatus: string = "timed-out";

    public wait(val: long, check: long, timeout?: long): Promise<string> {
        ConcurrencyHelpers.mutexLock(this.mutex);

        if (val != check) {
            ConcurrencyHelpers.mutexUnlock(this.mutex);
            return Promise.resolve<string>(Waiter.notEqualStatus);
        }
        if(!(timeout instanceof undefined)) {
            setTimeout((waiter: Waiter) => {
                waiter.notify(Waiter.timeOutStatus);
            }, timeout, this);
        }

        let resolver: Resolver | undefined = undefined;
        let p = new Promise<string>((resolve: Resolver) => {
            resolver = resolve;
        });
        this.promiseQueue.push(resolver!);
        ConcurrencyHelpers.mutexUnlock(this.mutex);
        return p;
    }

    public notify(val: string): boolean {
        ConcurrencyHelpers.mutexLock(this.mutex);
        let res = this.promiseQueue.poll();
        if(res instanceof null) {
            ConcurrencyHelpers.mutexUnlock(this.mutex);
            return false;
        }
        res!(val);
        ConcurrencyHelpers.mutexUnlock(this.mutex);
        return true;
    }
    
    private mutex: Object =  ConcurrencyHelpers.mutexCreate();
    private promiseQueue: BlockingQueue<Resolver> = new LinkedBlockingQueue<Resolver>();
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
    
    public static add(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: int, value: number): number {
        let buffer = typedArray.buffer
        if (typedArray instanceof Int8Array) {
            return buffer.atomicAddI8(index, value as byte)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicAddI16(index, value as short)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicAddI32(index, value as int)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicAddU8(index, value as byte)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicAddU16(index, value as short)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicAddU32(index, value as int)
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    public static add(typedArray: BigInt64Array | BigUint64Array, index: int, value: bigint): bigint {
        let buffer = typedArray.buffer
        if (typedArray instanceof BigUint64Array) {
            return new BigInt(buffer.atomicAddU64(index, value.getULong()))
        } else if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicAddI64(index, value.getLong()))
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    public static and(typedArray: Int8Array | Int16Array | Int32Array| Uint8Array | Uint16Array | Uint32Array, index: int, value: number): number {
        let buffer = typedArray.buffer
        if (typedArray instanceof Int8Array) {
            return buffer.atomicAndI8(index, value as byte)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicAndI16(index, value as short)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicAndI32(index, value as int)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicAndU8(index, value as byte)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicAndU16(index, value as short)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicAndU32(index, value as int)
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    public static and(typedArray: BigInt64Array | BigUint64Array, index: int, value: bigint): bigint {
        let buffer = typedArray.buffer
        if (typedArray instanceof BigUint64Array) {
            return new BigInt(buffer.atomicAndI64(index, value.getULong()))
        } else if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicAndU64(index, value.getLong()))
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    public static compareExchange(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: int, expectedValue: number, replacementValue: number): number {
        let buffer = typedArray.buffer
        if (typedArray instanceof Int8Array) {
            return buffer.atomicCompareExchangeI8(index, expectedValue as byte, replacementValue as byte)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicCompareExchangeI16(index, expectedValue as short, replacementValue as short)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicCompareExchangeI32(index, expectedValue as int, replacementValue as int)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicCompareExchangeU8(index, expectedValue as byte, replacementValue as byte)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicCompareExchangeU16(index, expectedValue as short, replacementValue as short)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicCompareExchangeU32(index, expectedValue as int, replacementValue as int)
        } else {
            throw new Error("Unhandled array type!")
        }
    }
    
    public static compareExchange(typedArray: BigInt64Array | BigUint64Array, index: int, expectedValue: bigint, replacementValue: bigint): bigint {
        let buffer = typedArray.buffer
        if (typedArray instanceof BigUint64Array) {
            return new BigInt(buffer.atomicCompareExchangeI64(index, expectedValue.getULong(), replacementValue.getULong()))
        } else if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicCompareExchangeU64(index, expectedValue.getLong(), replacementValue.getLong()))
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    public static exchange(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: int, value: number): number {
        let buffer = typedArray.buffer
        if (typedArray instanceof Int8Array) {
            return buffer.atomicExchangeI8(index, value as byte)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicExchangeI16(index, value as short)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicExchangeI32(index, value as int)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicExchangeU8(index, value as byte)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicExchangeU16(index, value as short)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicExchangeU32(index, value as int)
        } else {
            throw new Error("Unhandled array type!")
        }
    }
    
    public static exchange(typedArray: BigInt64Array | BigUint64Array, index: int, value: bigint): bigint {
        let buffer = typedArray.buffer
        if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicExchangeI64(index, value.getLong()))
        } else if (typedArray instanceof BigUint64Array) {
            return new BigInt(buffer.atomicExchangeU64(index, value.getULong()))
        } else {
            throw new Error("Unhandled array type!")
        }
    }
    
    public static load(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: int): number {
        let buffer = typedArray.buffer
        if (typedArray instanceof Int8Array) {
            return buffer.atomicLoadI8(index)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicLoadI16(index)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicLoadI32(index)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicLoadU8(index)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicLoadU16(index)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicLoadU32(index)
        } else {
            throw new Error("Unhandled array type!")
        }
    }


    public static load(typedArray: BigInt64Array, index: int): bigint {
        let buffer = typedArray.buffer
        return new BigInt(buffer.atomicLoadI64(index))
    }
    
    public static load(typedArray: BigUint64Array, index: int): bigint {
        let buffer = typedArray.buffer
        return new BigInt(buffer.atomicLoadU64(index))
    }

    public static or(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: int, value: number): number {
        let buffer = typedArray.buffer
        if (typedArray instanceof Int8Array) {
            return buffer.atomicOrI8(index, value as byte)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicOrI16(index, value as short)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicOrI32(index, value as int)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicOrU8(index, value as byte)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicOrU16(index, value as short)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicOrU32(index, value as int)
        } else {
            throw new Error("Unhandled array type!")
        }
    }
    
    public static or(typedArray: BigInt64Array | BigUint64Array, index: int, value: bigint): bigint {
        let buffer = typedArray.buffer
        if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicOrI64(index, value.getLong()))
        } else if (typedArray instanceof BigUint64Array) {
            return new BigInt(buffer.atomicOrU64(index, value.getULong()))
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    public static store(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: int, value: number): number {
        let buffer = typedArray.buffer
        if (typedArray instanceof Int8Array) {
            return buffer.atomicStoreI8(index, value as byte)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicStoreI16(index, value as short)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicStoreI32(index, value as int)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicStoreU8(index, value as byte)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicStoreU16(index, value as short)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicStoreU32(index, value as int)
        } else {
            throw new Error("Unhandled array type!")
        }
    }
    
    public static store(typedArray: BigInt64Array | BigUint64Array, index: int, value: bigint): bigint {
        let buffer = typedArray.buffer
        if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicStoreI64(index, value.getLong()))
        } else if (typedArray instanceof BigUint64Array) {
            return new BigInt(buffer.atomicStoreU64(index, value.getULong()))
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    public static sub(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: int, value: number): number {
        let buffer = typedArray.buffer
        if (typedArray instanceof Int8Array) {
            return buffer.atomicSubI8(index, value as byte)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicSubI16(index, value as short)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicSubI32(index, value as int)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicSubU8(index, value as byte)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicSubU16(index, value as short)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicSubU32(index, value as int)
        } else {
            throw new Error("Unhandled array type!")
        }
    }
    
    public static sub(typedArray: BigInt64Array | BigUint64Array, index: int, value: bigint): bigint {
        let buffer = typedArray.buffer
        if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicSubI64(index, value.getLong()))
        } else if (typedArray instanceof BigUint64Array) {
            return new BigInt(buffer.atomicSubU64(index, value.getULong()))
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    public static xor(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: int, value: number): number {
        let buffer = typedArray.buffer
        if (typedArray instanceof Int8Array) {
            return buffer.atomicXorI8(index, value as byte)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicXorI16(index, value as short)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicXorI32(index, value as int)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicXorU8(index, value as byte)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicXorU16(index, value as short)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicXorU32(index, value as int)
        } else {
            throw new Error("Unhandled array type!")
        }
    }
    
    public static xor(typedArray: BigInt64Array | BigUint64Array, index: int, value: bigint): bigint {
        let buffer = typedArray.buffer
        if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicXorI64(index, value.getLong()))
        } else if (typedArray instanceof BigUint64Array) {
            return new BigInt(buffer.atomicXorU64(index, value.getULong()))
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * If "typedArray[offset] != value" suspends the current thread until it is notified by Atomics.notify.
     *
     * Note: An Atomics.notify call will wake up this thread even if "typedArray[offset] == value".
     */
    public static wait(typedArray: Int32Array | BigInt64Array, offset: int, value: bigint): string {
        return await Atomics.waitAsync(typedArray, offset, value)
    }
    
    public async static waitAsync(typedArray: Int32Array | BigInt64Array, offset: int, value: bigint): Promise<string> {
        let buffer = typedArray.buffer
        if (typedArray instanceof Int32Array) {
            return Atomics.wait(buffer, offset, value.getLong(), (b: ArrayBuffer, offset: int) => { return b.atomicLoadI32(offset) as long; })
        } else {
            return Atomics.wait(buffer, offset, value.getLong(), (b: ArrayBuffer, offset: int) => { return buffer.atomicLoadI64(offset); })
        } 
    }


    /**
     * If "typedArray[offset] != value" suspends the current thread until it is notified by Atomics.notify
     * or until the given timeout passes.
     *
     * Note: An Atomics.notify call will wake up this thread even if "typedArray[offset] == value".
     */
    public static wait(typedArray: Int32Array | BigInt64Array, offset: int, value: bigint, timeout: long): string {
        return await Atomics.waitAsync(typedArray, offset, value, timeout);
    }
    
    public async static waitAsync(typedArray: Int32Array | BigInt64Array, offset: int, value: bigint, timeout: long): Promise<string> {
        let buffer = typedArray.buffer
        if (typedArray instanceof Int32Array) {
            return Atomics.wait(buffer, offset, value.getLong(), (b: ArrayBuffer, offset: int) => { return b.atomicLoadI32(offset) as long; }, timeout)
        } else {
            return Atomics.wait(buffer, offset, value.getLong(), (b: ArrayBuffer, offset: int) => { return buffer.atomicLoadI64(offset); }, timeout)
        } 
    }

    /**
     * Notifies (wakes up) threads that are suspended by the Atomics.wait() calls at the given index.
     * (index = typedArray.byteOffset + offset * sizeof(arrayElement))
     */
    public static notify(typedArray: Int32Array | BigInt64Array, offset: int): int {
        let buffer = typedArray.buffer
        if (typedArray instanceof Int32Array) {
            return Atomics.notify(buffer, offset)
        } else {
            return Atomics.notify(buffer, offset)
        }
    }

    /**
     * Operates exactly like Atomics.notify(Int32Array | BigInt64Array, int) but specifies the maximum number of threads to notify using 'count'.
     */
    public notify(typedArray: Int32Array | BigInt64Array, offset: int, count: int): int {
        let buffer = typedArray.buffer
        if (typedArray instanceof Int32Array) {
            return Atomics.notify(buffer, offset, count)
        } else {
            return Atomics.notify(buffer, offset, count)
        }
    }
    
    private static globalWaitingTable: ConcurrentHashMap<ArrayBuffer, ConcurrentHashMap<int, Waiter>> = new ConcurrentHashMap<ArrayBuffer, ConcurrentHashMap<int, Waiter>>();
    private static globalWaitingTableMutex = ConcurrencyHelpers.mutexCreate();

    private static wait(buffer: ArrayBuffer, byteOffset: int, waitValue: long, currentValueGetter: (b: ArrayBuffer, offset: int) => long, timeout?: long): Promise<string> {
        // Getting correct waiter
        let waiter = Atomics.getWaiter(buffer, byteOffset);
        // Check condition to wait
        let value = currentValueGetter(buffer, byteOffset);
        return waiter!.wait(value, waitValue, timeout);
    }

    private static notify(buffer: ArrayBuffer, byteOffset: int, count: int = 1): int {
        let waiter = Atomics.getWaiter(buffer, byteOffset);
        let notified = 0;
        for(let i = 0; i < count; i++) {
            if (waiter!.notify(Waiter.okStatus)) {
                notified++;
            }
        }
        return notified;
    }

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
    private static validateAtomicAccess(startByteOffset: int, elementSize: int, length: int, index: int): int {
        if (index < 0 || index >= length) {
            throw new RangeError("Index out of bounds")
        }
        return startByteOffset + (index * elementSize)
    }
}
