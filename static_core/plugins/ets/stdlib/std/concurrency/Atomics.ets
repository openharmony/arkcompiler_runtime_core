/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
 * Class enables atomic operations on a reference to an object of type T.
 * It provides thread-safe read and write operations without explicit synchronization.
 */
export final class AtomicReference<T> {
    constructor(ref: T) {
        this.ref_ = ref;
    }

    /**
     * Retrieves the current value of the atomic reference,
     *
     * @returns { T } The current value stored in the atomic reference
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native load(): T;

    /**
     * Sets the atomic reference to the given value.
     *
     * @param { T } - The new value to store in the atomic reference
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native store(ref: T): void;

    /**
     * Atomically sets the atomic reference to the given new value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { T } - The new value to store in the atomic reference
     * @returns { T } The previous value that was stored in the atomic reference before the update
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native exchange(ref: T): T;

    /**
     * Atomically sets the atomic reference to the given updated value if the current value
     * equals the expected value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { T } expected - The value that is expected to be currently stored in the atomic reference
     * @param { T } ref - The new value to store if the current value equals the expected value
     * @returns { T } the same value as expected if operation was successful (the value was changed),
     *                    correct old value if the current value did not match the expected value
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native compareAndSwap(expected: T, ref: T): T;
    
    /**
     * The method indicates whether the implementation of this structure is non-blocking, 
     * or whether it uses blocking primitives in the implementation.
     *
     * @return { boolean } True if the implementation is non-blocking
     *                    false if the implementation uses blocking sync primitives
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static native isLockFree(): boolean; 

    private ref_: T;
};

/**
 * Class enables atomic operations on an integer value.
 * It provides thread-safe read, write, and arithmetic operations without explicit synchronization.
 */
export final class AtomicInt {
    constructor(val: int) {
        this.val_ = val;
    }

    /**
     * Retrieves the current value of the atomic integer.
     *
     * @returns { int } The current value stored in the atomic integer
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native load(): int;

    /**
     * Sets the atomic integer to the given value.
     *
     * @param { int } val - The new value to store in the atomic integer
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native store(val: int): void;

    /**
     * Atomically sets the atomic integer to the given new value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { int } val - The new value to store in the atomic integer
     * @returns { int } The previous value that was stored in the atomic integer before the update
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native exchange(val: int): int;

    /**
     * Atomically sets the atomic integer to the given updated value if the current value
     * equals the expected value. 
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { int } expected - The value that is expected to be currently stored in the atomic integer
     * @param { int } val - The new value to store if the current value equals the expected value
     * @returns { int } the same value as expected if operation was successful (the value was changed),
     *                    correct old value if the current value did not match the expected value
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native compareAndSwap(expected: int, val: int): int;

    /**
     * Atomically adds the given value to the current value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { int } val - The value to add to the current atomic integer value
     * @returns { int } The previous value before the addition was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchAdd(val: int): int;
    
    /**
     * Atomically subtracts the given value from the current value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { int } val - The value to subtract from the current atomic integer value
     * @returns { int } The previous value before the subtraction was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchSub(val: int): int;

    /**
     * Atomically performs a bitwise AND operation between the current value and the given value,
     * stores the result, and returns the previous value.
     *
     * @param { int } val - The value to AND with the current atomic integer value
     * @returns { int } The previous value before the bitwise AND operation was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchAnd(val: int): int;

    /**
     * Atomically performs a bitwise OR operation between the current value and the given value,
     * stores the result, and returns the previous value.
     *
     * @param { int } val - The value to OR with the current atomic integer value
     * @returns { int } The previous value before the bitwise OR operation was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchOr(val: int): int;

    /**
     * Atomically performs a bitwise XOR operation between the current value and the given value,
     * stores the result, and returns the previous value.
     *
     * @param { int } val - The value to XOR with the current atomic integer value
     * @returns { int } The previous value before the bitwise XOR operation was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchXor(val: int): int;
    
    /**
     * The method indicates whether the implementation of this structure is non-blocking, 
     * or whether it uses blocking primitives in the implementation.
     *
     * @return { boolean } True if the implementation is non-blocking
     *                    false if the implementation uses blocking sync primitives
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static native isLockFree(): boolean; 

    private val_: int;
};

/**
 * Class enables atomic operations on a long value.
 * It provides thread-safe read, write, and arithmetic operations without explicit synchronization.
 */
export final class AtomicLong {
    constructor(val: long) {
        this.val_ = val;
    }

    /**
     * Retrieves the current value of the atomic long.
     *
     * @returns { long } The current value stored in the atomic long
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native load(): long;

    /**
     * Sets the atomic long to the given value.
     *
     * @param { long } val - The new value to store in the atomic long
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native store(val: long): void;

    /**
     * Atomically sets the atomic long to the given new value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { long } val - The new value to store in the atomic long
     * @returns { long } The previous value that was stored in the atomic long before the update
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native exchange(val: long): long;
    
    /**
     * Atomically sets the atomic long to the given updated value if the current value
     * equals the expected value. 
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { long } expected - The value that is expected to be currently stored in the atomic long
     * @param { long } val - The new value to store if the current value equals the expected value
     * @returns { long } the same value as expected if operation was successful (the value was changed),
     *                    correct old value if the current value did not match the expected value
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native compareAndSwap(expected: long, val: long): long;

    /**
     * Atomically adds the given value to the current value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { long } val - The value to add to the current atomic long value
     * @returns { long } The previous value before the addition was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchAdd(val: long): long;
    
    /**
     * Atomically subtracts the given value from the current value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { long } val - The value to subtract from the current atomic long value
     * @returns { long } The previous value before the subtraction was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchSub(val: long): long;
    
    /**
     * Atomically performs a bitwise AND operation between the current value and the given value,
     * stores the result, and returns the previous value.
     *
     * @param { long } val - The value to AND with the current atomic long value
     * @returns { long } The previous value before the bitwise AND operation was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchAnd(val: long): long;

    /**
     * Atomically performs a bitwise OR operation between the current value and the given value,
     * stores the result, and returns the previous value.
     *
     * @param { long } val - The value to OR with the current atomic long value
     * @returns { long } The previous value before the bitwise OR operation was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchOr(val: long): long;

    /**
     * Atomically performs a bitwise XOR operation between the current value and the given value,
     * stores the result, and returns the previous value.
     *
     * @param { long } val - The value to XOR with the current atomic long value
     * @returns { long } The previous value before the bitwise XOR operation was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchXor(val: long): long;
    
    /**
     * The method indicates whether the implementation of this structure is non-blocking, 
     * or whether it uses blocking primitives in the implementation.
     *
     * @return { boolean } True if the implementation is non-blocking
     *                    false if the implementation uses blocking sync primitives
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static native isLockFree(): boolean; 

    private val_: long;
};

/**
 * Class enables atomic operations on a short value.
 * It provides thread-safe read, write, and arithmetic operations without explicit synchronization.
 */
export final class AtomicShort {
    constructor(val: short) {
        this.val_ = val;
    }

    /**
     * Retrieves the current value of the atomic short.
     *
     * @returns { short } The current value stored in the atomic short
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native load(): short;

    /**
     * Sets the atomic short to the given value.
     *
     * @param { short } val - The new value to store in the atomic short
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native store(val: short): void;

    /**
     * Atomically sets the atomic short to the given new value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { short } val - The new value to store in the atomic short
     * @returns { short } The previous value that was stored in the atomic short before the update
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native exchange(val: short): short;
    
    /**
     * Atomically sets the atomic short to the given updated value if the current value
     * equals the expected value. 
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { short } expected - The value that is expected to be currently stored in the atomic short
     * @param { short } val - The new value to store if the current value equals the expected value
     * @returns { short } the same value as expected if operation was successful (the value was changed),
     *                    correct old value if the current value did not match the expected value
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native compareAndSwap(expected: short, val: short): short;

    /**
     * Atomically adds the given value to the current value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { short } val - The value to add to the current atomic short value
     * @returns { short } The previous value before the addition was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchAdd(val: short): short;
    
    /**
     * Atomically subtracts the given value from the current value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { short } val - The value to subtract from the current atomic short value
     * @returns { short } The previous value before the subtraction was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchSub(val: short): short;
    
    /**
     * Atomically performs a bitwise AND operation between the current value and the given value,
     * stores the result, and returns the previous value.
     *
     * @param { short } val - The value to AND with the current atomic short value
     * @returns { short } The previous value before the bitwise AND operation was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchAnd(val: short): short;

    /**
     * Atomically performs a bitwise OR operation between the current value and the given value,
     * stores the result, and returns the previous value.
     *
     * @param { short } val - The value to OR with the current atomic short value
     * @returns { short } The previous value before the bitwise OR operation was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchOr(val: short): short;

    /**
     * Atomically performs a bitwise XOR operation between the current value and the given value,
     * stores the result, and returns the previous value.
     *
     * @param { short } val - The value to XOR with the current atomic short value
     * @returns { short } The previous value before the bitwise XOR operation was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchXor(val: short): short;
    
    /**
     * The method indicates whether the implementation of this structure is non-blocking, 
     * or whether it uses blocking primitives in the implementation.
     *
     * @return { boolean } True if the implementation is non-blocking
     *                    false if the implementation uses blocking sync primitives
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static native isLockFree(): boolean; 

    private val_: short;
};

/**
 * Class enables atomic operations on a byte value.
 * It provides thread-safe read, write, and arithmetic operations without explicit synchronization.
 */
export final class AtomicByte {
    constructor(val: byte) {
        this.val_ = val;
    }

    /**
     * Retrieves the current value of the atomic byte.
     *
     * @returns { byte } The current value stored in the atomic byte
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native load(): byte;

    /**
     * Sets the atomic byte to the given value.
     *
     * @param { byte } val - The new value to store in the atomic byte
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native store(val: byte): void;

    /**
     * Atomically sets the atomic byte to the given new value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { byte } val - The new value to store in the atomic byte
     * @returns { byte } The previous value that was stored in the atomic byte before the update
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native exchange(val: byte): byte;
    
    /**
     * Atomically sets the atomic byte to the given updated value if the current value
     * equals the expected value. 
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { byte } expected - The value that is expected to be currently stored in the atomic byte
     * @param { byte } val - The new value to store if the current value equals the expected value
     * @returns { byte } the same value as expected if operation was successful (the value was changed),
     *                    correct old value if the current value did not match the expected value
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native compareAndSwap(expected: byte, val: byte): byte;

    /**
     * Atomically adds the given value to the current value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { byte } val - The value to add to the current atomic byte value
     * @returns { byte } The previous value before the addition was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchAdd(val: byte): byte;
    
    /**
     * Atomically subtracts the given value from the current value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { byte } val - The value to subtract from the current atomic byte value
     * @returns { byte } The previous value before the subtraction was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchSub(val: byte): byte;
    
    /**
     * Atomically performs a bitwise AND operation between the current value and the given value,
     * stores the result, and returns the previous value.
     *
     * @param { byte } val - The value to AND with the current atomic byte value
     * @returns { byte } The previous value before the bitwise AND operation was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchAnd(val: byte): byte;

    /**
     * Atomically performs a bitwise OR operation between the current value and the given value,
     * stores the result, and returns the previous value.
     *
     * @param { byte } val - The value to OR with the current atomic byte value
     * @returns { byte } The previous value before the bitwise OR operation was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchOr(val: byte): byte;

    /**
     * Atomically performs a bitwise XOR operation between the current value and the given value,
     * stores the result, and returns the previous value.
     *
     * @param { byte } val - The value to XOR with the current atomic byte value
     * @returns { byte } The previous value before the bitwise XOR operation was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native fetchXor(val: byte): byte;
    
    /**
     * The method indicates whether the implementation of this structure is non-blocking, 
     * or whether it uses blocking primitives in the implementation.
     *
     * @return { boolean } True if the implementation is non-blocking
     *                    false if the implementation uses blocking sync primitives
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static native isLockFree(): boolean; 

    private val_: byte;
};

/**
 * Class enables atomic operations on a float value.
 * It provides thread-safe read, write, and arithmetic operations without explicit synchronization.
 */
export final class AtomicFloat {
    constructor(val: float) {
        this.val_ = val;
    }

    /**
     * Retrieves the current value of the atomic float.
     *
     * @returns { float } The current value stored in the atomic float
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native load(): float;

    /**
     * Sets the atomic float to the given value.
     *
     * @param { float } val - The new value to store in the atomic float
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native store(val: float): void;

    /**
     * Atomically sets the atomic float to the given new value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { float } val - The new value to store in the atomic float
     * @returns { float } The previous value that was stored in the atomic float before the update
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native exchange(val: float): float;
    
    /**
     * Atomically sets the atomic float to the given updated value if the current value
     * equals the expected value. 
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { float } expected - The value that is expected to be currently stored in the atomic float
     * @param { float } val - The new value to store if the current value equals the expected value
     * @returns { float } the same value as expected if operation was successful (the value was changed),
     *                    correct old value if the current value did not match the expected value
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native compareAndSwap(expected: float, val: float): float;

    /**
     * Atomically adds the given value to the current value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { float } val - The value to add to the current atomic float value
     * @returns { float } The previous value before the addition was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public fetchAdd(val: float): float {
        let current: float = this.load();
        let expected: float = 0;
        do {
            expected = current;
            current = this.compareAndSwap(expected, expected + val);
        } while(current != expected)
        return expected;
    }
    
    /**
     * Atomically subtracts the given value from the current value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { float } val - The value to subtract from the current atomic float value
     * @returns { float } The previous value before the subtraction was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public fetchSub(val: float): float {
        let current: float = this.load();
        let expected: float = 0;
        do {
            expected = current;
            current = this.compareAndSwap(expected, expected - val);
        } while(current != expected)
        return expected;
    }
    
    /**
     * The method indicates whether the implementation of this structure is non-blocking, 
     * or whether it uses blocking primitives in the implementation.
     *
     * @return { boolean } True if the implementation is non-blocking
     *                    false if the implementation uses blocking sync primitives
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static native isLockFree(): boolean; 

    private val_: float;
};

/**
 * Class enables atomic operations on a double value.
 * It provides thread-safe read, write, and arithmetic operations without explicit synchronization.
 */
export final class AtomicDouble {
    constructor(val: double) {
        this.val_ = val;
    }

    /**
     * Retrieves the current value of the atomic double.
     *
     * @returns { double } The current value stored in the atomic double
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native load(): double;

    /**
     * Sets the atomic double to the given value.
     *
     * @param { double } val - The new value to store in the atomic double
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native store(val: double): void;

    /**
     * Atomically sets the atomic double to the given new value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { double } val - The new value to store in the atomic double
     * @returns { double } The previous value that was stored in the atomic double before the update
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native exchange(val: double): double;
    
    /**
     * Atomically sets the atomic double to the given updated value if the current value
     * equals the expected value. 
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { double } expected - The value that is expected to be currently stored in the atomic double
     * @param { double } val - The new value to store if the current value equals the expected value
     * @returns { double } the same value as expected if operation was successful (the value was changed),
     *                    correct old value if the current value did not match the expected value
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native compareAndSwap(expected: double, val: double): double;

    /**
     * Atomically adds the given value to the current value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { double } val - The value to add to the current atomic double value
     * @returns { double } The previous value before the addition was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public fetchAdd(val: double): double {
        let current: double = this.load();
        let expected: double = 0;
        do {
            expected = current;
            current = this.compareAndSwap(expected, expected + val);
        } while(current != expected)
        return expected;
    }
    
    /**
     * Atomically subtracts the given value from the current value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { double } val - The value to subtract from the current atomic double value
     * @returns { double } The previous value before the subtraction was performed
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public fetchSub(val: double): double {
        let current: double = this.load();
        let expected: double = 0;
        do {
            expected = current;
            current = this.compareAndSwap(expected, expected - val);
        } while(current != expected)
        return expected;
    }
    
    /**
     * The method indicates whether the implementation of this structure is non-blocking, 
     * or whether it uses blocking primitives in the implementation.
     *
     * @return { boolean } True if the implementation is non-blocking
     *                    false if the implementation uses blocking sync primitives
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static native isLockFree(): boolean; 

    private val_: double;
};

/**
 * Class enables atomic operations on a char value.
 * It provides thread-safe read, write without explicit synchronization.
 */
export final class AtomicChar {
    constructor(val: char) {
        this.val_ = val;
    }

    /**
     * Retrieves the current value of the atomic char.
     *
     * @returns { char } The current value stored in the atomic char
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native load(): char;

    /**
     * Sets the atomic char to the given value.
     *
     * @param { char } val - The new value to store in the atomic char
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native store(val: char): void;

    /**
     * Atomically sets the atomic char to the given new value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { char } val - The new value to store in the atomic char
     * @returns { char } The previous value that was stored in the atomic char before the update
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native exchange(val: char): char;
    
    /**
     * Atomically sets the atomic char to the given updated value if the current value
     * equals the expected value. 
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { char } expected - The value that is expected to be currently stored in the atomic char
     * @param { char } val - The new value to store if the current value equals the expected value
     * @returns { char } the same value as expected if operation was successful (the value was changed),
     *                    correct old value if the current value did not match the expected value
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native compareAndSwap(expected: char, val: char): char;
    
    /**
     * The method indicates whether the implementation of this structure is non-blocking, 
     * or whether it uses blocking primitives in the implementation.
     *
     * @return { boolean } True if the implementation is non-blocking
     *                    false if the implementation uses blocking sync primitives
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static native isLockFree(): boolean; 

    private val_: char;
};

/**
 * Class enables atomic operations on a boolean value.
 * It provides thread-safe read, write without explicit synchronization.
 */
export final class AtomicBoolean {
    constructor(val: boolean) {
        this.val_ = val;
    }

    /**
     * Retrieves the current value of the atomic boolean.
     *
     * @returns { boolean } The current value stored in the atomic boolean
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native load(): boolean;

    /**
     * Sets the atomic boolean to the given value.
     *
     * @param { boolean } val - The new value to store in the atomic boolean
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native store(val: boolean): void;

    /**
     * Atomically sets the atomic boolean to the given new value and returns the previous value.
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { boolean } val - The new value to store in the atomic boolean
     * @returns { boolean } The previous value that was stored in the atomic boolean before the update
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native exchange(val: boolean): boolean;
    
    /**
     * Atomically sets the atomic boolean to the given updated value if the current value
     * equals the expected value. 
     * This operation is performed as a single atomic action that cannot be interrupted by other threads.
     *
     * @param { boolean } expected - The value that is expected to be currently stored in the atomic boolean
     * @param { boolean } val - The new value to store if the current value equals the expected value
     * @returns { boolean } the same value as expected if operation was successful (the value was changed),
     *                    correct old value if the current value did not match the expected value
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public native compareAndSwap(expected: boolean, val: boolean): boolean;
    
    /**
     * The method indicates whether the implementation of this structure is non-blocking, 
     * or whether it uses blocking primitives in the implementation.
     *
     * @return { boolean } True if the implementation is non-blocking
     *                    false if the implementation uses blocking sync primitives
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static native isLockFree(): boolean; 

    private val_: boolean;
};
