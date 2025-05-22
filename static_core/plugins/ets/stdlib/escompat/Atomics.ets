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
     * Warning: currently, all Atomic operations use locks,
     * but isLockFree(1), isLockFree(2), isLockFree(4), isLockFree(8) following the ECMA specification return true.
     */
    public static isLockFree(byteSize: int): boolean {
        return byteSize == 1 || byteSize == 2 || byteSize == 4 || byteSize == 8
    }

    public static add(typedArray: Int8Array | Int16Array | Int32Array | BigInt64Array | Uint8Array | Uint16Array | Uint32Array | BigUint64Array, index: int, value: number): number {
        if (typedArray instanceof Int8Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int8Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue + value) as byte
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicAddI8(indexedPosition, value as byte)
            }
        } else if (typedArray instanceof Int16Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int16Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue + value) as short
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicAddI16(indexedPosition, value as short)
            }
        } else if (typedArray instanceof Int32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue + value) as int
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicAddI32(indexedPosition, value as int)
            }
        } else if (typedArray instanceof BigInt64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigInt64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition].getLong()
                let newValue = (oldValue + value) as long
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicAddI64(indexedPosition, value as long)
            }
        } else if (typedArray instanceof Uint8Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint8Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue + value) as byte
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicAddU8(indexedPosition, value as byte)
            }
        } else if (typedArray instanceof Uint16Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint16Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue + value) as short
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicAddU16(indexedPosition, value as short)
            }
        } else if (typedArray instanceof Uint32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue + value) as int
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicAddU32(indexedPosition, value as int)
            }
        } else if (typedArray instanceof BigUint64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigUint64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition].getULong()
                let newValue = (oldValue + value) as long
                typedArray.set(indexedPosition, new BigInt(newValue))
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicAddU64(indexedPosition, value as long)
            }
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    public static and(typedArray: Int8Array | Int16Array | Int32Array | BigInt64Array | Uint8Array | Uint16Array | Uint32Array | BigUint64Array, index: int, value: number): number {
        if (typedArray instanceof Int8Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int8Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue & value) as byte
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicAndI8(indexedPosition, value as byte)
            }
        } else if (typedArray instanceof Int16Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int16Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue & value) as short
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicAndI16(indexedPosition, value as short)
            }
        } else if (typedArray instanceof Int32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue & value) as int
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicAndI32(indexedPosition, value as int)
            }
        } else if (typedArray instanceof BigInt64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigInt64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition].getLong()
                let newValue = (oldValue & value) as long
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicAndI64(indexedPosition, value as long)
            }
        } else if (typedArray instanceof Uint8Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint8Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue & value) as byte
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicAndU8(indexedPosition, value as byte)
            }
        } else if (typedArray instanceof Uint16Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint16Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue & value) as short
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicAndU16(indexedPosition, value as short)
            }
        } else if (typedArray instanceof Uint32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue & value) as int
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicAndU32(indexedPosition, value as int)
            }
        } else if (typedArray instanceof BigUint64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigUint64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition].getLong()
                let newValue = (oldValue & value) as long
                typedArray.set(indexedPosition, new BigInt(newValue))
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicAndU64(indexedPosition, value as long)
            }
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    public static compareExchange(typedArray: Int8Array | Int16Array | Int32Array | BigInt64Array | Uint8Array | Uint16Array | Uint32Array | BigUint64Array, index: int, expectedValue: number, replacementValue: number): number {
        if (typedArray instanceof Int8Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int8Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                if (oldValue == expectedValue) {
                    typedArray.set(indexedPosition, replacementValue as byte)
                }
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicCompareExchangeI8(indexedPosition, expectedValue as byte, replacementValue as byte)
            }
        } else if (typedArray instanceof Int16Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int16Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                if (oldValue == expectedValue) {
                    typedArray.set(indexedPosition, replacementValue as short)
                }
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicCompareExchangeI16(indexedPosition, expectedValue as short, replacementValue as short)
            }
        } else if (typedArray instanceof Int32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                if (oldValue == expectedValue) {
                    typedArray.set(indexedPosition, replacementValue as int)
                }
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicCompareExchangeI32(indexedPosition, expectedValue as int, replacementValue as int)
            }
        } else if (typedArray instanceof BigInt64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigInt64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition].getLong()
                if (oldValue == expectedValue) {
                    typedArray.set(indexedPosition, replacementValue as long)
                }
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicCompareExchangeI64(indexedPosition, expectedValue as long, replacementValue as long)
            }
        } else if (typedArray instanceof Uint8Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint8Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                if (oldValue == expectedValue) {
                    typedArray.set(indexedPosition, replacementValue as byte)
                }
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicCompareExchangeU8(indexedPosition, expectedValue as byte, replacementValue as byte)
            }
        } else if (typedArray instanceof Uint16Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint16Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                if (oldValue == expectedValue) {
                    typedArray.set(indexedPosition, replacementValue as short)
                }
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicCompareExchangeU16(indexedPosition, expectedValue as short, replacementValue as short)
            }
        } else if (typedArray instanceof Uint32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                if (oldValue == expectedValue) {
                    typedArray.set(indexedPosition, replacementValue as int)
                }
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicCompareExchangeU32(indexedPosition, expectedValue as int, replacementValue as int)
            }
        } else if (typedArray instanceof BigUint64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigUint64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition].getLong()
                if (oldValue == expectedValue) {
                    typedArray.set(indexedPosition, new BigInt(replacementValue))
                }
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicCompareExchangeU64(indexedPosition, expectedValue as long, replacementValue as long)
            }
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    public static exchange(typedArray: Int8Array | Int16Array | Int32Array | BigInt64Array | Uint8Array | Uint16Array | Uint32Array | BigUint64Array, index: int, value: number): number {
        if (typedArray instanceof Int8Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int8Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                typedArray.set(indexedPosition, value as byte)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicExchangeI8(indexedPosition, value as byte)
            }
        } else if (typedArray instanceof Int16Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int16Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                typedArray.set(indexedPosition, value as short)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicExchangeI16(indexedPosition, value as short)
            }
        } else if (typedArray instanceof Int32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                typedArray.set(indexedPosition, value as int)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicExchangeI32(indexedPosition, value as int)
            }
        } else if (typedArray instanceof BigInt64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigInt64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition].getLong()
                typedArray.set(indexedPosition, value as long)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicExchangeI64(indexedPosition, value as long)
            }
        } else if (typedArray instanceof Uint8Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint8Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                typedArray.set(indexedPosition, value as byte)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicExchangeU8(indexedPosition, value as byte)
            }
        } else if (typedArray instanceof Uint16Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint16Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                typedArray.set(indexedPosition, value as short)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicExchangeU16(indexedPosition, value as short)
            }
        } else if (typedArray instanceof Uint32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                typedArray.set(indexedPosition, value as int)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicExchangeU32(indexedPosition, value as int)
            }
        } else if (typedArray instanceof BigUint64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigUint64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition].getLong()
                typedArray.set(indexedPosition, new BigInt(value))
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicExchangeU64(indexedPosition, value as long)
            }
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    public static load(typedArray: Int8Array | Int16Array | Int32Array | BigInt64Array | Uint8Array | Uint16Array | Uint32Array | BigUint64Array, index: int): number {
        if (typedArray instanceof Int8Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int8Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                return typedArray[indexedPosition]
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicLoadI8(indexedPosition)
            }
        } else if (typedArray instanceof Int16Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int16Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                return typedArray[indexedPosition]
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicLoadI16(indexedPosition)
            }
        } else if (typedArray instanceof Int32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                return typedArray[indexedPosition]
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicLoadI32(indexedPosition)
            }
        } else if (typedArray instanceof BigInt64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigInt64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                return typedArray[indexedPosition].getLong()
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicLoadI64(indexedPosition)
            }
        } else if (typedArray instanceof Uint8Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint8Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                return typedArray[indexedPosition]
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicLoadU8(indexedPosition)
            }
        } else if (typedArray instanceof Uint16Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint16Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                return typedArray[indexedPosition]
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicLoadU16(indexedPosition)
            }
        } else if (typedArray instanceof Uint32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                return typedArray[indexedPosition]
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicLoadU32(indexedPosition)
            }
        } else if (typedArray instanceof BigUint64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigUint64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                return typedArray[indexedPosition].getLong()
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicLoadU64(indexedPosition)
            }
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    public static or(typedArray: Int8Array | Int16Array | Int32Array | BigInt64Array | Uint8Array | Uint16Array | Uint32Array | BigUint64Array, index: int, value: number): number {
        if (typedArray instanceof Int8Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int8Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue | value) as byte
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicOrI8(indexedPosition, value as byte)
            }
        } else if (typedArray instanceof Int16Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int16Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue | value) as short
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicOrI16(indexedPosition, value as short)
            }
        } else if (typedArray instanceof Int32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue | value) as int
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicOrI32(indexedPosition, value as int)
            }
        } else if (typedArray instanceof BigInt64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigInt64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition].getLong()
                let newValue = (oldValue | value) as long
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicOrI64(indexedPosition, value as long)
            }
        } else if (typedArray instanceof Uint8Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint8Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue | value) as byte
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicOrU8(indexedPosition, value as byte)
            }
        } else if (typedArray instanceof Uint16Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint16Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue | value) as short
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicOrU16(indexedPosition, value as short)
            }
        } else if (typedArray instanceof Uint32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue | value) as int
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicOrU32(indexedPosition, value as int)
            }
        } else if (typedArray instanceof BigUint64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigUint64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition].getLong()
                let newValue = (oldValue | value) as long
                typedArray.set(indexedPosition, new BigInt(newValue))
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicOrU64(indexedPosition, value as long)
            }
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    public static store(typedArray: Int8Array | Int16Array | Int32Array | BigInt64Array | Uint8Array | Uint16Array | Uint32Array | BigUint64Array, index: int, value: number): number {
        if (typedArray instanceof Int8Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int8Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                typedArray.set(indexedPosition, value as byte)
                return value
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicStoreI8(indexedPosition, value as byte)
            }
        } else if (typedArray instanceof Int16Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int16Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                typedArray.set(indexedPosition, value as short)
                return value
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicStoreI16(indexedPosition, value as short)
            }
        } else if (typedArray instanceof Int32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                typedArray.set(indexedPosition, value as int)
                return value
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicStoreI32(indexedPosition, value as int)
            }
        } else if (typedArray instanceof BigInt64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigInt64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                typedArray.set(indexedPosition, value as long)
                return value
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicStoreI64(indexedPosition, value as long)
            }
        } else if (typedArray instanceof Uint8Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint8Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                typedArray.set(indexedPosition, value as byte)
                return value
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicStoreU8(indexedPosition, value as byte)
            }
        } else if (typedArray instanceof Uint16Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint16Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                typedArray.set(indexedPosition, value as short)
                return value
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicStoreU16(indexedPosition, value as short)
            }
        } else if (typedArray instanceof Uint32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                typedArray.set(indexedPosition, value as int)
                return value
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicStoreU32(indexedPosition, value as int)
            }
        } else if (typedArray instanceof BigUint64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigUint64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                typedArray.set(indexedPosition, new BigInt(value))
                return value
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicStoreU64(indexedPosition, value as long)
            }
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    public static sub(typedArray: Int8Array | Int16Array | Int32Array | BigInt64Array | Uint8Array | Uint16Array | Uint32Array | BigUint64Array, index: int, value: number): number {
        if (typedArray instanceof Int8Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int8Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue - value)
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicSubI8(indexedPosition, value as byte)
            }
        } else if (typedArray instanceof Int16Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int16Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue - value)
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicSubI16(indexedPosition, value as short)
            }
        } else if (typedArray instanceof Int32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue - value)
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicSubI32(indexedPosition, value as int)
            }
        } else if (typedArray instanceof BigInt64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigInt64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition].getLong()
                let newValue = (oldValue - value)
                typedArray.set(indexedPosition, newValue as long)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicSubI64(indexedPosition, value as long)
            }
        } else if (typedArray instanceof Uint8Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint8Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue - value)
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicSubU8(indexedPosition, value as byte)
            }
        } else if (typedArray instanceof Uint16Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint16Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue - value)
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicSubU16(indexedPosition, value as short)
            }
        } else if (typedArray instanceof Uint32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue - value)
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicSubU32(indexedPosition, value as int)
            }
        } else if (typedArray instanceof BigUint64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigUint64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition].getLong()
                let newValue = (oldValue - value)
                typedArray.set(indexedPosition, new BigInt(newValue))
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicSubU64(indexedPosition, value as long)
            }
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    public static xor(typedArray: Int8Array | Int16Array | Int32Array | BigInt64Array | Uint8Array | Uint16Array | Uint32Array | BigUint64Array, index: int, value: number): number {
        if (typedArray instanceof Int8Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int8Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue ^ value)
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicXorI8(indexedPosition, value as byte)
            }
        } else if (typedArray instanceof Int16Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int16Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue ^ value)
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicXorI16(indexedPosition, value as short)
            }
        } else if (typedArray instanceof Int32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue ^ value)
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicXorI32(indexedPosition, value as int)
            }
        } else if (typedArray instanceof BigInt64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigInt64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition].getLong()
                let newValue = (oldValue ^ value)
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicXorI64(indexedPosition, value as long)
            }
        } else if (typedArray instanceof Uint8Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint8Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue ^ value)
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicXorU8(indexedPosition, value as byte)
            }
        } else if (typedArray instanceof Uint16Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint16Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue ^ value)
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicXorU16(indexedPosition, value as short)
            }
        } else if (typedArray instanceof Uint32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Uint32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition]
                let newValue = (oldValue ^ value)
                typedArray.set(indexedPosition, newValue)
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicXorU32(indexedPosition, value as int)
            }
        } else if (typedArray instanceof BigUint64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigUint64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, index)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                let oldValue = typedArray[indexedPosition].getLong()
                let newValue = (oldValue ^ value)
                typedArray.set(indexedPosition, new BigInt(newValue))
                return oldValue
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicXorU64(indexedPosition, value as long)
            }
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * If "typedArray[offset] != value" suspends the current thread until it is notified by Atomics.notify.
     *
     * Note: An Atomics.notify call will wake up this thread even if "typedArray[offset] == value".
     */
    public static wait(typedArray: Int32Array | BigInt64Array, offset: int, value: number): string {
        if (typedArray instanceof Int32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, offset)
            let mem = Atomics.requireSharedMemory(typedArray.buffer)
            let result = mem.atomicWaitI32(indexedPosition, value as int)
            return Atomics.interpretWaitResult(result)
        } else if (typedArray instanceof BigInt64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigInt64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, offset)
            let mem = Atomics.requireSharedMemory(typedArray.buffer)
            let result = mem.atomicWaitI64(indexedPosition, value as long)
            return Atomics.interpretWaitResult(result)
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    public static waitAsync(typedArray: Int32Array | BigInt64Array, offset: int, value: number): Promise<string> {
        return launch Atomics.wait(typedArray, offset, value)
    }

    /**
     * If "typedArray[offset] != value" suspends the current thread until it is notified by Atomics.notify
     * or until the given timeout passes.
     *
     * Note: An Atomics.notify call will wake up this thread even if "typedArray[offset] == value".
     */
    public static wait(typedArray: Int32Array | BigInt64Array, offset: int, value: number, timeout: long): string {
        if (typedArray instanceof Int32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, offset)
            let mem = Atomics.requireSharedMemory(typedArray.buffer)
            let result = mem.atomicTimedWaitI32(indexedPosition, value as int, timeout)
            return Atomics.interpretWaitResult(result)
        } else if (typedArray instanceof BigInt64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigInt64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, offset)
            let mem = Atomics.requireSharedMemory(typedArray.buffer)
            let result = mem.atomicTimedWaitI64(indexedPosition, value as long, timeout)
            return Atomics.interpretWaitResult(result)
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    public static waitAsync(typedArray: Int32Array | BigInt64Array, offset: int, value: number, timeout: long): Promise<string> {
        return launch Atomics.wait(typedArray, offset, value, timeout)
    }

    /**
     * Notifies (wakes up) threads that are suspended by the Atomics.wait() calls at the given index.
     * (index = typedArray.byteOffset + offset * sizeof(arrayElement))
     */
    public static notify(typedArray: Int32Array | BigInt64Array, offset: int): int {
        if (typedArray instanceof Int32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, offset)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                return 0
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicNotify(indexedPosition)
            }
        } else if (typedArray instanceof BigInt64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigInt64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, offset)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                return 0
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicNotify(indexedPosition)
            }
        } else {
            throw new Error("Unhandled array type!")
        }
    }

    /**
     * Operates exactly like Atomics.notify(Int32Array | BigInt64Array, int) but specifies the maximum number of threads to notify using 'count'.
     */
    public static notify(typedArray: Int32Array | BigInt64Array, offset: int, count: int): int {
        if (typedArray instanceof Int32Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, Int32Array.BYTES_PER_ELEMENT as int, typedArray.length as int, offset)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                return 0
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicBoundedNotify(indexedPosition, count)
            }
        } else if (typedArray instanceof BigInt64Array) {
            let indexedPosition = Atomics.validateAtomicAccess(typedArray.byteOffset as int, BigInt64Array.BYTES_PER_ELEMENT as int, typedArray.length as int, offset)
            let buffer = typedArray.buffer
            if (buffer instanceof ArrayBuffer) {
                return 0
            } else {
                let mem = (buffer as SharedArrayBuffer).getSharedMemory()
                return mem.atomicBoundedNotify(indexedPosition, count)
            }
        } else {
            throw new Error("Unhandled array type!")
        }
    }
}
