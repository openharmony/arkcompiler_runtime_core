/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
package std.concurrency;

/**
 * Basic atomic operations
 */
export namespace Atomics {
    const INFINITY: int = Int.MAX_VALUE;

    /**
     * Checks if atomics implementation uses hardware atomic instructions instead of locks
     *
     * Warning: currently, all Atomic operations use locks,
     * but isLockFree(1), isLockFree(2), isLockFree(4), isLockFree(8) following the ECMA specification return true.
     *
     * @param { int } byteSize size of memory localion
     * @returns { boolean } true if Atomic operations for typed arrays where "BYTER_PER_ELEMENT == byteSize"
     *     use hardware atomics instructions instead of locks.
     */
    export function isLockFree(byteSize: int): boolean {
        return byteSize == 1 || byteSize == 2 || byteSize == 4 || byteSize == 8
    }

    /**
     * Atomically adds value to current value and replaces it with the result
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @param { long } value value to add to current value
     * @returns { long } current value before the operation
     */
    export function add(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: int, value: long): long {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt();
        if (typedArray instanceof Int8Array) {
            return buffer.atomicAddI8(index, byteOffset, value.toByte())
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicAddI16(index, byteOffset, value.toShort())
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicAddI32(index, byteOffset, value.toInt())
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicAddU8(index, byteOffset, value.toByte())
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicAddU16(index, byteOffset, value.toShort())
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicAddU32(index, byteOffset, value.toInt())
        } else {
            throw new Error('Unhandled array type')
        }
    }

    /**
     * Atomically adds value to current value and replaces it with the result
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @param { bigint } value value to add to current value
     * @returns { bigint } current value before the operation
     */
    export function add(typedArray: BigInt64Array | BigUint64Array, index: int, value: bigint): bigint {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        if (typedArray instanceof BigUint64Array) {
            return BigInt.fromULong(buffer.atomicAddU64(index, byteOffset, value.getULong()))
        } else if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicAddI64(index, byteOffset, value.getLong()))
        } else {
            throw new Error('Unhandled array type')
        }
    }

    /**
     * Atomically replaces current value it with the result of bitwise AND with value
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @param { long } value value to perform bitwise AND with current value
     * @returns { long } current value before the operation
     */
    export function and(typedArray: Int8Array | Int16Array | Int32Array| Uint8Array | Uint16Array | Uint32Array, index: int, value: long): long {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        if (typedArray instanceof Int8Array) {
            return buffer.atomicAndI8(index, byteOffset, value.toByte())
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicAndI16(index, byteOffset, value.toShort())
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicAndI32(index, byteOffset, value.toInt())
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicAndU8(index, byteOffset, value.toByte())
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicAndU16(index, byteOffset, value.toShort())
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicAndU32(index, byteOffset, value.toInt())
        } else {
            throw new Error('Unhandled array type')
        }
    }

    /**
     * Atomically replaces current value it with the result of bitwise AND with value
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @param { bigint } value value to perform bitwise AND with current value
     * @returns { bigint } current value before the operation
     */
    export function and(typedArray: BigInt64Array | BigUint64Array, index: int, value: bigint): bigint {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        if (typedArray instanceof BigUint64Array) {
            return BigInt.fromULong(buffer.atomicAndU64(index, byteOffset, value.getULong()))
        } else if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicAndI64(index, byteOffset, value.getLong()))
        } else {
            throw new Error('Unhandled array type')
        }
    }

    /**
     * Performs a compare and swap operation with the location from the typed array
     * Atomically compares current value with the expected value. It they're equal, replaces
     * current value with the new value, else does nothing
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @param { long } expectedValue expected value for the current value
     * @param { long } replacementValue value to replace current value with in case of success
     * @returns { long } current value before the operation
     */
    export function compareExchange(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: int, expectedValue: long, replacementValue: long): long {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        if (typedArray instanceof Int8Array) {
            return buffer.atomicCompareExchangeI8(index, byteOffset, expectedValue.toByte(), replacementValue.toByte())
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicCompareExchangeI16(index, byteOffset, expectedValue.toShort(), replacementValue.toShort())
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicCompareExchangeI32(index, byteOffset, expectedValue.toInt(), replacementValue.toInt())
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicCompareExchangeU8(index, byteOffset, expectedValue.toByte(), replacementValue.toByte())
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicCompareExchangeU16(index, byteOffset, expectedValue.toShort(), replacementValue.toShort())
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicCompareExchangeU32(index, byteOffset, expectedValue.toInt(), replacementValue.toInt())
        } else {
            throw new Error('Unhandled array type')
        }
    }

    /**
     * Performs a compare and swap operation with the location from the typed array
     * Atomically compares current value with the expected value
     * If they're equal, replaces current value with the new value
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @param { bigint } expectedValue expected value for the current value
     * @param { bigint } replacementValue value to replace current value with in case of success
     * @returns { bigint } current value before the operation
     */
    export function compareExchange(typedArray: BigInt64Array | BigUint64Array, index: int, expectedValue: bigint, replacementValue: bigint): bigint {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        if (typedArray instanceof BigUint64Array) {
            return BigInt.fromULong(buffer.atomicCompareExchangeU64(index, byteOffset, expectedValue.getULong(), replacementValue.getULong()))
        } else if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicCompareExchangeI64(index, byteOffset, expectedValue.getLong(), replacementValue.getLong()))
        } else {
            throw new Error('Unhandled array type')
        }
    }

    /**
     * Performs an exchange operation with the location from the typed array
     * Atomically compares current value with the expected value
     * If they're equal, replaces current value with the new value
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @param { long } value value to replace current value with
     * @returns { long } current value before the operation
     */
    export function exchange(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: int, value: long): long {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        if (typedArray instanceof Int8Array) {
            return buffer.atomicExchangeI8(index, byteOffset, value.toByte())
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicExchangeI16(index, byteOffset, value.toShort())
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicExchangeI32(index, byteOffset, value.toInt())
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicExchangeU8(index, byteOffset, value.toByte())
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicExchangeU16(index, byteOffset, value.toShort())
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicExchangeU32(index, byteOffset, value.toInt())
        } else {
            throw new Error('Unhandled array type')
        }
    }

    /**
     * Performs an exchange operation with the location from the typed array
     * Atomically replaces current value with the new value
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @param { bigint } value value to replace current value with
     * @returns { bigint } current value before the operation
     */
    export function exchange(typedArray: BigInt64Array | BigUint64Array, index: int, value: bigint): bigint {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicExchangeI64(index, byteOffset, value.getLong()))
        } else if (typedArray instanceof BigUint64Array) {
            return BigInt.fromULong(buffer.atomicExchangeU64(index, byteOffset, value.getULong()))
        } else {
            throw new Error('Unhandled array type')
        }
    }

    /**
     * Atomically reads value from the typed array
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @returns { long } current value
     */
    export function load(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: int): long {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        if (typedArray instanceof Int8Array) {
            return buffer.atomicLoadI8(index, byteOffset)
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicLoadI16(index, byteOffset)
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicLoadI32(index, byteOffset)
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicLoadU8(index, byteOffset)
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicLoadU16(index, byteOffset)
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicLoadU32(index, byteOffset)
        } else {
            throw new Error('Unhandled array type')
        }
    }

    /**
     * Atomically reads value from the typed array
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @returns { bigint } current value
     */
    export function load(typedArray: BigInt64Array, index: int): bigint {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        return new BigInt(buffer.atomicLoadI64(index, byteOffset))
    }

    /**
     * Atomically reads value from the typed array
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @returns { bigint } current value
     */
    export function load(typedArray: BigUint64Array, index: int): bigint {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        return BigInt.fromULong(buffer.atomicLoadU64(index, byteOffset))
    }

    /**
     * Atomically replaces current value it with the result of bitwise OR with value
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @param { long } value value to perform bitwise OR with current value
     * @returns { long } current value before the operation
     */
    export function or(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: int, value: long): long {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        if (typedArray instanceof Int8Array) {
            return buffer.atomicOrI8(index, byteOffset, value.toByte())
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicOrI16(index, byteOffset, value.toShort())
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicOrI32(index, byteOffset, value.toInt())
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicOrU8(index, byteOffset, value.toByte())
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicOrU16(index, byteOffset, value.toShort())
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicOrU32(index, byteOffset, value.toInt())
        } else {
            throw new Error('Unhandled array type')
        }
    }

    /**
     * Atomically replaces current value it with the result of bitwise OR with value
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @param { bigint } value value to perform bitwise OR with current value
     * @returns { bigint } current value before the operation
     */
    export function or(typedArray: BigInt64Array | BigUint64Array, index: int, value: bigint): bigint {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicOrI64(index, byteOffset, value.getLong()))
        } else if (typedArray instanceof BigUint64Array) {
            return BigInt.fromULong(buffer.atomicOrU64(index, byteOffset, value.getULong()))
        } else {
            throw new Error('Unhandled array type')
        }
    }

    /**
     * Atomically writes new value into the typed array
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @param { long } value new value
     * @returns { long } new value
     */
    export function store(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: int, value: long): long {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        if (typedArray instanceof Int8Array) {
            return buffer.atomicStoreI8(index, byteOffset, value.toByte())
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicStoreI16(index, byteOffset, value.toShort())
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicStoreI32(index, byteOffset, value.toInt())
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicStoreU8(index, byteOffset, value.toByte())
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicStoreU16(index, byteOffset, value.toShort())
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicStoreU32(index, byteOffset, value.toInt())
        } else {
            throw new Error('Unhandled array type')
        }
    }

    /**
     * Atomically writes new value into the typed array
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @param { bigint } value new value
     * @returns { bigint } new value
     */
    export function store(typedArray: BigInt64Array | BigUint64Array, index: int, value: bigint): bigint {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicStoreI64(index, byteOffset, value.getLong()))
        } else if (typedArray instanceof BigUint64Array) {
            return BigInt.fromULong(buffer.atomicStoreU64(index, byteOffset, value.getULong()))
        } else {
            throw new Error('Unhandled array type')
        }
    }

    /**
     * Atomically subtracts value from current value and replaces it with the result
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @param { long } value value to subtract from current value
     * @returns { long } current value before the operation
     */
    export function sub(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: int, value: long): long {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        if (typedArray instanceof Int8Array) {
            return buffer.atomicSubI8(index, byteOffset, value.toByte())
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicSubI16(index, byteOffset, value.toShort())
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicSubI32(index, byteOffset, value.toInt())
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicSubU8(index, byteOffset, value.toByte())
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicSubU16(index, byteOffset, value.toShort())
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicSubU32(index, byteOffset, value.toInt())
        } else {
            throw new Error('Unhandled array type')
        }
    }

    /**
     * Atomically subtracts value from current value and replaces it with the result
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @param { bigint } value value to subtract from current value
     * @returns { bigint } current value before the operation
     */
    export function sub(typedArray: BigInt64Array | BigUint64Array, index: int, value: bigint): bigint {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicSubI64(index, byteOffset, value.getLong()))
        } else if (typedArray instanceof BigUint64Array) {
            return BigInt.fromULong(buffer.atomicSubU64(index, byteOffset, value.getULong()))
        } else {
            throw new Error('Unhandled array type')
        }
    }

    /**
     * Atomically replaces current value it with the result of bitwise XOR with value
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @param { long } value value to perform bitwise XOR with current value
     * @returns { long } current value before the operation
     */
    export function xor(typedArray: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array, index: int, value: long): long {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        if (typedArray instanceof Int8Array) {
            return buffer.atomicXorI8(index, byteOffset, value.toByte())
        } else if (typedArray instanceof Int16Array) {
            return buffer.atomicXorI16(index, byteOffset, value.toShort())
        } else if (typedArray instanceof Int32Array) {
            return buffer.atomicXorI32(index, byteOffset, value.toInt())
        } else if (typedArray instanceof Uint8Array) {
            return buffer.atomicXorU8(index, byteOffset, value.toByte())
        } else if (typedArray instanceof Uint16Array) {
            return buffer.atomicXorU16(index, byteOffset, value.toShort())
        } else if (typedArray instanceof Uint32Array) {
            return buffer.atomicXorU32(index, byteOffset, value.toInt())
        } else {
            throw new Error('Unhandled array type')
        }
    }

    /**
     * Atomically replaces current value it with the result of bitwise XOR with value
     *
     * @param typedArray typed array of values
     * @param  { int } index index of current value in the typedArray
     * @param { bigint } value value to perform bitwise XOR with current value
     * @returns { bigint } current value before the operation
     */
    export function xor(typedArray: BigInt64Array | BigUint64Array, index: int, value: bigint): bigint {
        validateIndex(typedArray, index);
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        if (typedArray instanceof BigInt64Array) {
            return new BigInt(buffer.atomicXorI64(index, byteOffset, value.getLong()))
        } else if (typedArray instanceof BigUint64Array) {
            return BigInt.fromULong(buffer.atomicXorU64(index, byteOffset, value.getULong()))
        } else {
            throw new Error('Unhandled array type')
        }
    }

    /**
     * If "typedArray[index] == value" suspends the current thread until it is notified by Atomics.notify.
     *
     * Note: An Atomics.notify call will wake up this thread even if "typedArray[index] == value".
     *
     * @param typedArray typed array of values
     * @param { int } index index of current value in the typedArray
     * @param { long } value value to compare current value with
     * @returns { string } status "ok", "not-equal" or "timed-out"
     */
    export function wait(typedArray: Int32Array, index: int, value: long): string {
        validateIndex(typedArray, index);
        if (Atomics.load(typedArray, index) != value) {
            return 'not-equal';
        }
        return waitImpl(typedArray, index, value.toLong());
    }

    /**
     * If "typedArray[index] == value" suspends the current thread until it is notified by Atomics.notify
     * or until the given timeout passes.
     *
     * Note: An Atomics.notify call will wake up this thread even if "typedArray[index] == value".
     *
     * @param typedArray typed array of values
     * @param { int } index index of current value in the typedArray
     * @param { long } value value to compare current value with
     * @param { int } timeout timeout in ms
     * @returns { string } status "ok", "not-equal" or "timed-out"
     */
    export function wait(typedArray: Int32Array, index: int, value: long, timeout: int): string {
        validateIndex(typedArray, index);
        if (Atomics.load(typedArray, index) != value) {
            return 'not-equal';
        }
        return waitImpl(typedArray, index, value.toLong(), timeout);
    }

    /**
     * If "typedArray[index] == value" suspends the current thread until it is notified by Atomics.notify.
     *
     * Note: An Atomics.notify call will wake up this thread even if "typedArray[index] == value".
     *
     * @param typedArray typed array of values
     * @param { int } index index of current value in the typedArray
     * @param { bigint } value value to compare current value with
     * @returns { string } status "ok", "not-equal" or "timed-out"
     */
    export function wait(typedArray: BigInt64Array, index: int, value: bigint): string {
        validateIndex(typedArray, index);
        if (Atomics.load(typedArray, index) != value) {
            return 'not-equal';
        }
        return waitImpl(typedArray, index, value.getLong());
    }

    /**
     * If "typedArray[index] == value" suspends the current thread until it is notified by Atomics.notify.
     * or until the given timeout passes.
     *
     * Note: An Atomics.notify call will wake up this thread even if "typedArray[index] == value".
     *
     * @param typedArray typed array of values
     * @param { int } index index of current value in the typedArray
     * @param { bigint } value value to compare current value with
     * @param { int } timeout timeout in ms
     * @returns { string } status "ok", "not-equal" or "timed-out"
     */
    export function wait(typedArray: BigInt64Array, index: int, value: bigint, timeout: int): string {
        validateIndex(typedArray, index);
        if (Atomics.load(typedArray, index) != value) {
            return 'not-equal';
        }
        return waitImpl(typedArray, index, value.getLong(), timeout);
    }

    /**
     * Creates a promise that will be pending if "typedArray[index] == value" until it is notified by Atomics.notify
     *
     * Note: An Atomics.notify call will resolve this promise even if "typedArray[index] == value".
     *
     * @param typedArray typed array of values
     * @param { int } index index of current value in the typedArray
     * @param { long } value value to compare current value with
     * @returns { Promise<string> } that will be resolved after the coroutine get notified
     *    or rejected if exception were thrown
     */
    export function waitAsync(typedArray: Int32Array, index: int, value: long): Promise<string> {
        validateIndex(typedArray, index);
        if (Atomics.load(typedArray, index) != value) {
            return Promise.resolve('not-equal');
        }
        return asyncWaitImpl(typedArray, index, value.toLong()) as Promise<string>;
    }

    /**
     * Creates a promise that will be pending if "typedArray[index] == value" until it is notified by Atomics.notify
     *
     * Note: An Atomics.notify call will resolve this promise even if "typedArray[index] == value".
     *
     * @param typedArray typed array of values
     * @param { int } index index of current value in the typedArray
     * @param { bigint } value value to compare current value with
     * @returns { Promise<string> } that will be resolved after the coroutine get notified
     *    or rejected if exception were thrown
     */
    export function waitAsync(typedArray: BigInt64Array, index: int, value: bigint): Promise<string> {
        validateIndex(typedArray, index);
        if (Atomics.load(typedArray, index) != value) {
            return Promise.resolve('not-equal');
        }
        return asyncWaitImpl(typedArray, index, value.getLong()) as Promise<string>;
    }

    /**
     * Creates a promise that will be pending if "typedArray[index] == value" until it is notified by Atomics.notify
     * or until the given timeout passes.
     *
     * Note: An Atomics.notify call will resolve this promise even if "typedArray[index] == value".
     *
     * @param typedArray typed array of values
     * @param { int } index index of current value in the typedArray
     * @param { long } value value to compare current value with
     * @param { int } timeout timeout in ms
     * @returns { Promise<string> } that will be resolved after the coroutine get notified
     *    or rejected if exception were thrown
     */
    export function waitAsync(typedArray: Int32Array, index: int, value: long, timeout: int): Promise<string> {
        validateIndex(typedArray, index);
        if (Atomics.load(typedArray, index) != value) {
            return Promise.resolve('not-equal');
        }
        return asyncWaitImpl(typedArray, index, value.toLong(), timeout) as Promise<string>;
    }

    /**
     * Creates a promise that will be pending if "typedArray[index] == value" until it is notified by Atomics.notify
     * or until the given timeout passes.
     *
     * Note: An Atomics.notify call will resolve this promise even if "typedArray[index] == value".
     *
     * @param typedArray typed array of values
     * @param { int } index index of current value in the typedArray
     * @param { bigint } value value to compare current value with
     * @param { int } timeout timeout in ms
     * @returns { Promise<string> } that will be resolved after the coroutine get notified
     *    or rejected if exception were thrown
     */
    export function waitAsync(typedArray: BigInt64Array, index: int, value: bigint, timeout: int): Promise<string> {
        validateIndex(typedArray, index);
        if (Atomics.load(typedArray, index) != value) {
            return Promise.resolve('not-equal');
        }
        return asyncWaitImpl(typedArray, index, value.getLong(), timeout) as Promise<string>;
    }

    /**
     * Notifies (wakes up) threads that are suspended by the Atomics.wait() calls at the given index.
     * (index = typedArray.byteOffset + offset * sizeof(arrayElement))
     *
     * @param typedArray typed array of values
     * @param { int } index index in the typed array to read current value from
     * @returns { int } number of notified threads
     */
    export function notify(typedArray: Int32Array | BigInt64Array, index: int): int {
        validateIndex(typedArray, index);
        return notifyImpl(typedArray, index)
    }

    /**
     * Operates exactly like Atomics.notify(Int32Array | BigInt64Array, int) but specifies the maximum number of threads to notify using 'count'.
     *
     * @param typedArray typed array of values
     * @param { int } index index in the typed array to read current value from
     * @param { int } count maximum number of threads to notify
     * @returns { int } number of notified threads
     */
    export function notify(typedArray: Int32Array | BigInt64Array, index: int, count: int): int {
        validateIndex(typedArray, index);
        return notifyImpl(typedArray, index, count)
    }

    function waitImpl(typedArray: Int32Array | BigInt64Array, index: int, value: long, timeout?: int): string {
        let buffer = typedArray.buffer
        let byteOffset = typedArray.byteOffset.toInt()
        if (timeout === undefined) {
            timeout = INFINITY;
        } else if (timeout < 0) {
            timeout = 0;
        }
        if (typedArray instanceof Int32Array) {
            let offset = index2byteOffset(byteOffset, Int32Array.BYTES_PER_ELEMENT, typedArray.length, index)
            return waitImplInt32(buffer, offset, value.toInt(), timeout);
        } else {
            let offset = index2byteOffset(byteOffset, BigInt64Array.BYTES_PER_ELEMENT, typedArray.length, index)
            return waitImplInt64(buffer, offset, value, timeout);
        }
    }

    native function waitImplInt32(buffer: ArrayBuffer, index: int, value: int, timeout: int): string;
    native function waitImplInt64(buffer: ArrayBuffer, index: int, value: long, timeout: int): string;

    function asyncWaitImpl(typedArray: Int32Array | BigInt64Array, index: int, value: long, timeout?: int): Promise<string> {
        let buffer = typedArray.buffer;
        let byteOffset = typedArray.byteOffset.toInt();
        if (timeout === undefined) {
            timeout = INFINITY;
        } else if (timeout < 0) {
            timeout = 0;
        }
        if (typedArray instanceof Int32Array) {
            let offset = index2byteOffset(byteOffset, Int32Array.BYTES_PER_ELEMENT, typedArray.length, index)
            return asyncWaitImplInt32(buffer, offset, value.toInt(), timeout);
        } else {
            let offset = index2byteOffset(byteOffset, BigInt64Array.BYTES_PER_ELEMENT, typedArray.length, index)
            return asyncWaitImplInt64(buffer, offset, value, timeout);
        }
    }

    native function asyncWaitImplInt32(buffer: ArrayBuffer, index: int, value: int, timeout: int): Promise<string>;
    native function asyncWaitImplInt64(buffer: ArrayBuffer, index: int, value: long, timeout: int): Promise<string>;

    function notifyImpl(typedArray: Int32Array | BigInt64Array, index: int, count?: int): int {
        let buffer = typedArray.buffer
        let offset = 0;
        let byteOffset = typedArray.byteOffset.toInt()
        let icount: int = 0;
        if (count === undefined) {
            icount = INFINITY;
        } else {
            icount = count;
        }
        if (typedArray instanceof Int32Array) {
            offset = index2byteOffset(byteOffset, Int32Array.BYTES_PER_ELEMENT, typedArray.length, index)
            return notifyImplInt32(buffer, offset, icount);
        } else {
            offset = index2byteOffset(byteOffset, BigInt64Array.BYTES_PER_ELEMENT, typedArray.length, index)
            return notifyImplInt64(buffer, offset, icount);
        }
    }

    native function notifyImplInt32(buffer: ArrayBuffer, index: int, count: int): int;
    native function notifyImplInt64(buffer: ArrayBuffer, index: int, count: int): int;

    // https://tc39.es/ecma262/multipage/structured-data.html#sec-validateatomicaccess
    function index2byteOffset(startByteOffset: int, elementSize: int, length: int, index: int): int {
        if (index < 0 || index >= length) {
            throw new RangeError("Index out of bounds")
        }
        return startByteOffset + (index * elementSize)
    }

    function validateIndex(typedArray: Int8Array | Int16Array | Int32Array | BigInt64Array | Uint8Array | Uint16Array | Uint32Array | BigUint64Array, index: int) {
        if (typedArray instanceof Int8Array) {
            let notUnionTypedArray = typedArray as Int8Array;
            if (index < 0 || index >= notUnionTypedArray.length) {
                throw new RangeError("Index out of bounds")
            }
        } else if (typedArray instanceof Int16Array) {
            let notUnionTypedArray = typedArray as Int16Array;
            if (index < 0 || index >= notUnionTypedArray.length) {
                throw new RangeError("Index out of bounds")
            }
        } else if (typedArray instanceof Int32Array) {
            let notUnionTypedArray = typedArray as Int32Array;
            if (index < 0 || index >= notUnionTypedArray.length) {
                throw new RangeError("Index out of bounds")
            }
        } else if (typedArray instanceof BigInt64Array) {
            let notUnionTypedArray = typedArray as BigInt64Array;
            if (index < 0 || index >= notUnionTypedArray.length) {
                throw new RangeError("Index out of bounds")
            }
        } else if (typedArray instanceof Uint8Array) {
            let notUnionTypedArray = typedArray as Uint8Array;
            if (index < 0 || index >= notUnionTypedArray.length) {
                throw new RangeError("Index out of bounds")
            }
        } else if (typedArray instanceof Uint16Array) {
            let notUnionTypedArray = typedArray as Uint16Array;
            if (index < 0 || index >= notUnionTypedArray.length) {
                throw new RangeError("Index out of bounds")
            }
        } else if (typedArray instanceof Uint32Array) {
            let notUnionTypedArray = typedArray as Uint32Array;
            if (index < 0 || index >= notUnionTypedArray.length) {
                throw new RangeError("Index out of bounds")
            }
        } else if (typedArray instanceof BigUint64Array) {
            let notUnionTypedArray = typedArray as BigUint64Array;
            if (index < 0 || index >= notUnionTypedArray.length) {
                throw new RangeError("Index out of bounds")
            }
        } else {
            throw new Error("Unhandled array type");
        }
    }
}
