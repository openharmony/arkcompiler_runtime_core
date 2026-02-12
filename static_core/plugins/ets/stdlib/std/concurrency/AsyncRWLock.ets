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
 * The concurrency.stackless module provides APIs for stackless coroutines.
 *
 * @namespace concurrency.stackless
 * @syscap SystemCapability.Utils.Lang
 * @since 24
 */
export namespace concurrency.stackless {

/**
 * Base interface for async lock operations.
 *
 * @interface AsyncBaseLock
 * @syscap SystemCapability.Utils.Lang
 * @since 24
 */
export interface AsyncBaseLock {
    lock(): Promise<void>;
    unlock();
}

/**
 * Interface for read-write lock operations.
 *
 * @interface AsyncRWLock
 * @syscap SystemCapability.Utils.Lang
 * @since 24
 */
export interface AsyncRWLock {
    readLock(): AsyncReadLock;
    writeLock(): AsyncWriteLock;
}

/**
 * State management for read-write lock encoding.
 * Encodes lock state, reader count, and writer count in a single atomic long value.
 *
 * @syscap SystemCapability.Utils.Lang
 * @since 24
 */
class State {

    /**
     * Checks if the lock is in unlocked state.
     *
     * @param { long } state Current lock state
     * @returns { boolean } True if the lock is unlocked, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static IsUnlocked(state: long): boolean {
        return state == State.UNLOCKED;
    }

    /**
     * Checks if the lock is held by readers.
     *
     * @param { long } state Current lock state
     * @returns { boolean } True if the lock is held by readers, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static HasReadLock(state: long): boolean {
        return (state & State.STATE_MASK) == State.READ_LOCKED;
    }

    /**
     * Checks if the lock is held by a writer.
     *
     * @param { long } state Current lock state
     * @returns { boolean } True if the lock is held by a writer, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static HasWriteLock(state: long): boolean {
        return (state & State.STATE_MASK) == State.WRITE_LOCKED;
    }

    /**
     * Checks if there are any waiting readers.
     *
     * @param { long } state Current lock state
     * @returns { boolean } True if there are waiting readers, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static HasReaders(state: long): boolean {
        return (state & State.READERS_MASK) != 0;
    }

    /**
     * Checks if there are any waiting writers.
     *
     * @param { long } state Current lock state
     * @returns { boolean } True if there are waiting writers, false otherwise
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static HasWriters(state: long): boolean {
        return (state & State.WRITERS_MASK) != 0;
    }

    /**
     * Gets the number of waiting readers.
     *
     * @param { long } state Current lock state
     * @returns { long } Number of waiting readers
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static GetReaders(state: long): long {
        return (state & State.READERS_MASK) >> State.STATE_BITS;
    }

    /**
     * Increments the reader count in the state.
     *
     * @param { long } state Current lock state
     * @returns { long } Updated state value with incremented reader count
     * @throws { RangeError } If the reader count would overflow the maximum allowed value
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static IncReaders(state: long): long {
        if ((state & State.READERS_MASK) == State.READERS_MASK) {
            throw new RangeError("AsyncReadWriteLock:: Too many readers");
        }
        return state + State.ONE_READER;
    }

    /**
     * Computes the new state when a reader attempts to acquire the lock.
     * Increments the reader count and, if there is no writer lock, sets READ_LOCKED.
     *
     * @param { long } state Current lock state
     * @returns { long } Updated state with reader count and lock state encoded
     * @throws { RangeError } If the reader count would overflow the maximum allowed value
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static PrepareReadLock(state: long): long {
        return State.IncReaders(state) | (State.HasWriteLock(state) ? 0 : State.READ_LOCKED);
    }

    /**
     * Increments the writer count in the state.
     *
     * @param { long } state Current lock state
     * @returns { long } Updated state value with incremented writer count
     * @throws { RangeError } If the writer count would overflow the maximum allowed value
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static IncWriters(state: long): long {
        if ((state & State.WRITERS_MASK) == State.WRITERS_MASK) {
            throw new RangeError("AsyncReadWriteLock:: Too many writers");
        }
        return state + State.ONE_WRITER;
    }

    /**
     * Computes the new state when a writer attempts to acquire the lock.
     * Increments the writer count and, if there is no read lock, sets WRITE_LOCKED.
     *
     * @param { long } state Current lock state
     * @returns { long } Updated state with writer count and lock state encoded
     * @throws { RangeError } If the writer count would overflow the maximum allowed value
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static PrepareWriteLock(state: long): long {
        return State.IncWriters(state) | (State.HasReadLock(state) ? 0 : State.WRITE_LOCKED);
    }

    /**
     * Decrements reader count and clears lock state.
     *
     * @param { long } state Current lock state
     * @returns { long } Updated state value with decremented reader count
     * @throws { RangeError } If the reader count would underflow
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static DecReadersClearState(state: long): long {
        if ((state & State.READERS_MASK) == 0) {
            throw new RangeError("AsyncReadWriteLock:: Too few readers");
        }
        return (state - State.ONE_READER) & ~State.STATE_MASK;
    }

    /**
     * Decrements writer count and clears lock state.
     *
     * @param { long } state Current lock state
     * @returns { long } Updated state value with decremented writer count
     * @throws { RangeError } If the writer count would underflow
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static DecWritersClearState(state: long): long {
        if ((state & State.WRITERS_MASK) == 0) {
            throw new RangeError("AsyncReadWriteLock:: Too few writers");
        }
        return (state - State.ONE_WRITER) & ~State.STATE_MASK;
    }

    /**
     * Applies the appropriate lock state (READ_LOCKED, WRITE_LOCKED, or UNLOCKED)
     * based on the presence of waiting readers or writers.
     *
     * @param { long } state Current lock state with counters updated
     * @returns { long } State with lock bits updated according to readers and writers
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public static ApplyLockState(state: long): long {
        return state | (State.HasReaders(state) ? State.READ_LOCKED :
            State.HasWriters(state) ? State.WRITE_LOCKED : State.UNLOCKED);
    }


    // Bit layout (64 bits):
    //   [63 ... 33] [32 ... 2] [1  0]
    //   writer(31)  reader(31) state(2)
    public static readonly BASEBIT: long = 1;
    public static readonly STATE_BITS: long = 2;
    public static readonly READERS_BITS: long = 31;

    public static readonly UNLOCKED: long = 0;
    public static readonly READ_LOCKED: long = 1;
    public static readonly WRITE_LOCKED: long = 2;
    public static readonly STATE_MASK: long = 3;

    // 31 bits set (bits 0-30) = 0x7FFFFFFF
    public static readonly FIELD_MASK: long = (State.BASEBIT << State.READERS_BITS) - State.BASEBIT;
    // bits 2-32:  0x1FFFFFFFC
    public static readonly READERS_MASK: long = State.FIELD_MASK << State.STATE_BITS;
    // bits 33-63: 0xFFFFFFFE00000000
    public static readonly WRITERS_MASK: long = State.FIELD_MASK << (State.STATE_BITS + State.READERS_BITS);

    public static readonly ONE_READER: long = State.BASEBIT << State.STATE_BITS;
    public static readonly ONE_WRITER: long = State.BASEBIT << (State.STATE_BITS + State.READERS_BITS);
}

/**
 * Coroutine read-write lock that allows multiple readers or a single writer.
 * Multiple readers can hold the lock simultaneously, but writers have exclusive access.
 *
 * @class AsyncReadWriteLock
 * @implements AsyncRWLock
 * @syscap SystemCapability.Utils.Lang
 * @since 24
 */
export class AsyncReadWriteLock implements AsyncRWLock {
    constructor() {
        this.rLock_ = new AsyncReadLock(this);
        this.wLock_ = new AsyncWriteLock(this);
        this.readers_ = new WaitersList();
        this.writers_ = new WaitersList();
        this.state_ = new AtomicLong(State.UNLOCKED);
    }

    /**
     * Gets the read lock instance for acquiring read locks.
     *
     * @returns { AsyncReadLock } Read lock instance
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public readLock(): AsyncReadLock {
        return this.rLock_;
    }

    /**
     * Gets the write lock instance for acquiring write locks.
     *
     * @returns { AsyncWriteLock } Write lock instance
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public writeLock(): AsyncWriteLock {
        return this.wLock_;
    }

    /**
     * Acquires a read lock. Blocks if a writer holds the lock.
     *
     * @returns { Promise<void> } A promise that resolves when the read lock is acquired
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public async executeReadLock(): Promise<void> {
        let oldState = this.state_.load();
        let newState: long = State.PrepareReadLock(oldState);
        while (this.state_.compareAndSwap(oldState, newState) != oldState) {
            oldState = this.state_.load();
            newState = State.PrepareReadLock(oldState);
        }

        if (!State.HasReadLock(newState)) {
            await this.readers_.suspend();
        }
    }

    /**
     * Acquires a write lock. Blocks if any readers or a writer hold the lock.
     *
     * Reader preference implementation:
     * - New readers can still acquire the lock while a writer is waiting,
     *   as long as no writer currently holds the lock.
     * - If there is no active read lock, the state is switched to WRITE_LOCKED
     *   so that the writer can acquire the lock immediately.
     * - On unlock, waiting readers are woken before waiting writers.
     *
     * @returns { Promise<void> } A promise that resolves when the write lock is acquired
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public async executeWriteLock(): Promise<void> {
        let oldState = this.state_.load();
        let newState: long = State.PrepareWriteLock(oldState);
        while (this.state_.compareAndSwap(oldState, newState) != oldState) {
            oldState = this.state_.load();
            newState = State.PrepareWriteLock(oldState);
        }
        
        if (!State.HasWriteLock(newState) || State.HasWriteLock(oldState)) {
            await this.writers_.suspend();
        }
    }

    /**
     * Releases the lock, allowing other coroutines to acquire it.
     * Wakes up waiting writers or readers as appropriate.
     *
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public unlock(): void {
        if (State.IsUnlocked(this.state_.load())) {
            throw new IllegalLockStateError();
        }

        let oldState = this.state_.load();
        let newState: long = State.HasReadLock(oldState) ? State.DecReadersClearState(oldState) : State.DecWritersClearState(oldState);
        newState = State.ApplyLockState(newState);
        while (this.state_.compareAndSwap(oldState, newState) != oldState) {
            oldState = this.state_.load();
            newState = State.HasReadLock(oldState) ? State.DecReadersClearState(oldState) : State.DecWritersClearState(oldState);
            newState = State.ApplyLockState(newState);
        }

        if (State.IsUnlocked(newState) || (State.HasReadLock(oldState) && State.HasReadLock(newState))) {
            return;
        }

        if (State.HasWriteLock(newState)) {
            this.writers_.resume();
            return;
        }

        for (let readers = State.GetReaders(newState); readers != 0; readers--) {
            this.readers_.resume();
        }
    }




    private rLock_: AsyncReadLock;
    private wLock_: AsyncWriteLock;

    private readers_: WaitersList;
    private writers_: WaitersList;
    private state_: AtomicLong;
}

/**
 * Read lock guard for acquiring and releasing read locks.
 *
 * @class AsyncReadLock
 * @implements AsyncBaseLock
 * @syscap SystemCapability.Utils.Lang
 * @since 24
 */
export class AsyncReadLock implements AsyncBaseLock {
    constructor(rwl: AsyncReadWriteLock) {
        this.rwl_ = rwl;
    }

    /**
     * Acquires the read lock. Blocks if a writer holds the lock.
     *
     * @returns { Promise<void> } A promise that resolves when the read lock is acquired
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public async lock(): Promise<void> {
        await this.rwl_.executeReadLock();
    }

    /**
     * Releases the read lock.
     *
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public unlock() {
        this.rwl_.unlock();
    }

    private rwl_: AsyncReadWriteLock;
}

/**
 * Write lock guard for acquiring and releasing write locks.
 *
 * @class AsyncWriteLock
 * @implements AsyncBaseLock
 * @syscap SystemCapability.Utils.Lang
 * @since 24
 */
export class AsyncWriteLock implements AsyncBaseLock {
    constructor(rwl: AsyncReadWriteLock) {
        this.rwl_ = rwl;
    }

    /**
     * Acquires the write lock. Blocks if any readers or a writer hold the lock.
     *
     * @returns { Promise<void> } A promise that resolves when the write lock is acquired
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public async lock(): Promise<void> {
        await this.rwl_.executeWriteLock();
    }

    /**
     * Releases the write lock.
     *
     * @syscap SystemCapability.Utils.Lang
     * @since 24
     */
    public unlock() {
        this.rwl_.unlock();
    }

    private rwl_: AsyncReadWriteLock;
}

}
