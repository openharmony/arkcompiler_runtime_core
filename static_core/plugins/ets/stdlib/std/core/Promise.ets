/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
 * Interface for objects with 'then' method.
 */
export interface PromiseLike<out T> {
    then<U = T, E = never>(onFulfilled: (value: T) => U|PromiseLike<U>,
            onRejected?: (error: Error) => E|PromiseLike<E>): PromiseLike<U|E>;
}

export class PromiseStatus{
    static readonly fulfilled = "fulfilled";
    static readonly rejected = "rejected";
}

/**
 * Base class for Promise.allSettled return type.
 */
class PromiseSettledResultBase {
    status: string = "";
}

/**
 * The class describes fulfillment result of Promise.allSettled's argument.
 */
export class PromiseFulfilledResult<T> extends PromiseSettledResultBase {
    constructor(){}

    constructor(value: T) {
        this.status = PromiseStatus.fulfilled;
        this.value = value;
    }

    value: T;
}

/**
 * The class describes rejection result of Promise.allSettled's argument.
 */
export class PromiseRejectedResult extends PromiseSettledResultBase {
    constructor(){
        this.reason =  new Error();
    }

    constructor(reason: Error) {
        this.status = PromiseStatus.rejected;
        this.reason = reason;
    }

    reason: Error;
}

export type PromiseSettledResult<T> = PromiseFulfilledResult<T>|PromiseRejectedResult

/**
 * Class represents a result of an asynchronous operation in the future.
 */
export final class Promise<out T> implements PromiseLike<T> {
    private constructor() {
    }

    constructor(callback: (resolve: (value: T|PromiseLike<T>) => void,
            reject: (error: Error) => void) => void) {
        try {
            callback((value: T|PromiseLike<T>): void => {
                this.doResolve<T>(value);
            }, (error: Error): void => {
                this.rejectImpl(error as Error, false);
            });
        } catch (error: Error) {
            this.rejectImpl(error, false);
        }
    }

    then<U = T>(onFulfilled: () => U|PromiseLike<U>): Promise<U> {
        let promise = new Promise<U>();
        let fn: () => void = (): void => {
            try {
                if (this.state == Promise.STATE_RESOLVED) {
                    promise.doResolve<U>(onFulfilled());
                } else {
                    promise.rejectImpl(this.value as Error, false);
                }
            } catch (error: Error) {
                promise.rejectImpl(error, false);
            }
        }
        this.submitCallback(fn as Object);
        return promise;
    }

    then<U = T, E = never>(onFulfilled: (value: T) => U|PromiseLike<U>,
            onRejected?: (error: Error) => E|PromiseLike<E>): Promise<U|E> {
        let promise = new Promise<U|E>();
        let fn: () => void = (): void => {
            try {
                if (this.state == Promise.STATE_RESOLVED) {
                    promise.doResolve<U>(onFulfilled(this.value as T));
                } else {
                    if (onRejected) {
                        promise.doResolve<E>(onRejected(this.value as Error));
                    } else {
                        promise.rejectImpl(this.value as Error, false);
                    }
                }
            } catch (e: Error) {
                promise.rejectImpl(e, false);
            }
        }
        this.submitCallback(fn as Object);
        return promise;
    }

    // NOTE(audovichenko): Add '?' to onRejection param. Issue #20001
    catch<U = never>(onRejected: () => U|PromiseLike<U>): Promise<T|U> {
        let promise = new Promise<T|U>();
        let fn: () => void = (): void => {
            try {
                if (this.state == Promise.STATE_REJECTED) {
                    let res: U|PromiseLike<U> = onRejected();
                    promise.doResolve<U>(res);
                } else {
                    promise.doResolve<T>(this.value as T);
                }
            } catch (e: Error) {
                promise.rejectImpl(e, false);
            }
        }
        this.submitCallback(fn as Object);
        return promise;
    }

    catch<U = never>(onRejected?: (error: Error) => U|PromiseLike<U>): Promise<T|U> {
        let promise = new Promise<T|U>();
        let fn: () => void = (): void => {
            try {
                if (this.state == Promise.STATE_REJECTED) {
                    if (onRejected) {
                        let res: U|PromiseLike<U> = onRejected(this.value as Error);
                        promise.doResolve<U>(res);
                    } else {
                        promise.rejectImpl(this.value as Error, false);
                    }
                } else {
                    promise.doResolve<T>(this.value as T);
                }
            } catch (e: Error) {
                promise.rejectImpl(e, false);
            }
        }
        this.submitCallback(fn as Object);
        return promise;
    }

    finally(onFinally?: () => void): Promise<T> {
        let promise = new Promise<T>();
        let fn: () => void = (): void => {
            try {
                if (onFinally) {
                    try {
                        onFinally();
                    } catch(error: Error) {
                        promise.rejectImpl(error, false);
                    }
                }
                if (this.state == Promise.STATE_RESOLVED) {
                    promise.resolveImpl(this.value as T, false);
                } else {
                    promise.rejectImpl(this.value as Error, false);
                }
            } catch (e: Error) {
                promise.rejectImpl(e, false);
            }
        }
        this.submitCallback(fn as Object);
        return promise;
    }

    static resolve(): Promise<void> {
        let p = new Promise<void>();
        p.resolveImpl(undefined, false);
        return p;
    }

    static resolve<U>(value: U|PromiseLike<U>): Promise<U> {
        if (value instanceof Promise) {
            return value as Promise<U>;
        }
        let p = new Promise<U>();
        p.doResolve(value);
        return p;
    }

    static reject(): Promise<void> {
        let p = new Promise<void>();
        p.rejectImpl(new Error(), false);
        return p;
    }

    static reject<U = never>(error: Error): Promise<U> {
        let p = new Promise<U>();
        p.rejectImpl(error, false);
        return p;
    }

    static all<U>(promises: FixedArray<U|PromiseLike<U>|undefined>): Promise<Array<U>> {
        return new Promise<Array<U>>((resolve: (value: Array<U>) => void, reject: (error: Error) => void): void => {
            new AllPromiseConcurrency<U>(resolve, reject).all(promises);
        });
    }

    static all<U>(promises: Iterable<U|PromiseLike<U>>): Promise<Array<U>> {
        try {
            return Promise.all<U>(Promise.toArray(promises));
        } catch (e: Error) {
            return Promise.reject<Array<U>>(e);
        }
    }

    static allSettled<U>(promises: FixedArray<U|PromiseLike<U>|undefined>): Promise<PromiseSettledResult<U>[]> {
        return new Promise<PromiseSettledResult<U>[]>((resolve: (value: PromiseSettledResult<U>[]) => void, reject: (error: Error) => void): void => {
            // This temp object is needed because es2panda cannot change captured primitives
            new AllSettledPromiseConcurrency<U>(resolve, reject).allSettled(promises);
        });
    }

    static allSettled<U>(promises: Iterable<U|PromiseLike<U>>): Promise<PromiseSettledResult<U>[]> {
        try {
            return Promise.allSettled<U>(Promise.toArray(promises));
        } catch (e: Error) {
            return Promise.reject<PromiseSettledResult<U>[]>(e);
        }
    }

    static any<U>(promises: FixedArray<U|PromiseLike<U>|undefined>): Promise<U> {
        return new Promise<U>((resolve: (value: U) => void, reject: (error: Error) => void): void => {
            // This temp object is needed because es2panda cannot change captured primitives
            new AnyPromiseConcurrency<U>(resolve, reject).any(promises);
        });
    }

    static any<U>(promises: Iterable<U|PromiseLike<U>>): Promise<U> {
        try {
            return Promise.any<U>(Promise.toArray(promises));
        } catch (e: Error) {
            return Promise.reject<U>(e);
        }
    }

    static race<U>(promises: FixedArray<U|PromiseLike<U>|undefined>): Promise<U> {
        return new Promise<U>((resolve: (value: U) => void, reject: (error: Error) => void): void => {
            // This temp object is needed because es2panda cannot change captured primitives
            new RacePromiseConcurrency<U>(resolve, reject).race(promises);
        });
    }

    static race<U>(promises: Iterable<U|PromiseLike<U>>): Promise<U> {
        try {
            return Promise.race<U>(Promise.toArray(promises));
        } catch (e: Error) {
            return Promise.reject<U>(e);
        }
    }

    private static toArray<U>(values: Iterable<U>): FixedArray<U | undefined> {
        let it = values.$_iterator();
        let v = it.next();
        if (v.done) {
            return new (U | undefined)[0];
        }
        let array : FixedArray<U | undefined> = new (U | undefined)[1];
        array[0] = v.value!;
        let len = 1;
        while (true) {
            v = it.next();
            if (v.done) {
                return Promise.trim(array, len);
            }
            array = Promise.ensureCapacity(array, len + 1);
            array[len] = v.value!;
            ++len;
        }
    }

    private static trim<U>(array: FixedArray<U>, length: int): FixedArray<U | undefined> {
        if (array.length == length) {
            return array;
        }
        let result : FixedArray<U | undefined> = new (U | undefined)[length];
        for (let i = 0; i < length; ++i) {
            result[i] = array[i];
        }
        return result;
    }

    private static ensureCapacity<U>(queue: FixedArray<U>, size: int): FixedArray<U | undefined> {
        if (size <= queue.length) {
            return queue;
        }
        let newQueue : FixedArray<U | undefined> = new (U | undefined)[size * 2];
        for (let i = 0; i < queue.length; ++i) {
            newQueue[i] = queue[i];
        }
        return newQueue;
    }

    private subscribeOnAnotherPromise<U>(internalPromise: PromiseLike<U>): void {
        this.mutex.lock();
        if(this.state != Promise.STATE_PENDING) {
            this.mutex.unlock();
            return;
        }
        this.state = Promise.STATE_LINKED;
        this.mutex.unlock();
        let thisPromise = this;
        let fn: () => void = ():void => {
            try{
                internalPromise.then<void, void>((value: U): void => {
                    thisPromise.resolveImpl(value, true);
                }, (error: Error): void => {
                    thisPromise.rejectImpl(error, true);
                });
            } catch(error: Error) {
                thisPromise.rejectImpl(error, true);
            }
        }
        if(internalPromise instanceof Promise) {
            fn();
        } else {
            // To guarantee that the then method of a thenable executes asynchronously.
            Promise.resolve().then(fn);
        }
    }

    private doResolve<U>(value: U|PromiseLike<U>): void {
        if (value instanceof PromiseLike) {
            if (value === this) {
                this.rejectImpl(new TypeError("Cannot reolve the promise by self"), false);
            }
            this.subscribeOnAnotherPromise<U>(value as PromiseLike<U>);
        } else {
            this.resolveImpl(value, false);
        }
    }

    public native awaitResolution(): T;

    private native resolveImpl<U>(value: U, wasLinked: boolean): void;
    private native rejectImpl(error: Object, wasLinked: boolean): void;
    private native submitCallback(callback: Object): void;

    private static STATE_PENDING = 0;
    private static STATE_LINKED = 1;
    private static STATE_RESOLVED = 2;
    private static STATE_REJECTED = 3;
    private static EMPTY_QUEUE = new Object[0];

    // Order of fields should be the following
    // 1. Reference fields
    // 2. Primitive fields in mostly size decreasing order
    // filling alignment holes
    private value: NullishType = undefined;
    private mutex = new Mutex();
    private event = new Event();
    private callbackQueue?: Object[];
    private launchModeQueue?: int[];
    private interopObject?: Object;
    private linkedPromise?: Object;
    private queueSize: int = 0;
    private state: int = Promise.STATE_PENDING;
}

class PromiseConcurrencyBase<T> {
    toPromiseLike(value: T|PromiseLike<T>): PromiseLike<T> {
        if (value instanceof Promise) {
            let p = value as Promise<T>;
            return p;
        } else {
            let p = Promise.resolve<T>(value as T);
            return p;
        }
    }
}

class AllPromiseConcurrency<T> extends PromiseConcurrencyBase<T> {
    constructor(resolve: (value: Array<T>) => void, reject: (error: Error) => void) {
        this.resolve = resolve;
        this.reject = reject;
    }

    all(promises: FixedArray<T|PromiseLike<T>|undefined>): void {
        if (promises.length == 0) {
            this.resolve(this.values);
            return;
        } else {
            this.values = new Array<T>(promises.length);
        }
        for (let i = 0; i < promises.length; ++i) {
            let idx = i;
            this.toPromiseLike(promises[idx]!).then<void, void>((value: T): void => {
                this.resolveImpl(value, idx);
            }, (error: Error): void => {
                this.reject(error as Error);
            });
        }
    }

    private resolveImpl(value: T, idx: int): void {
        this.values[idx] = value;
        ++this.resolvedCnt;
        if (this.resolvedCnt == this.values!.length) {
            this.resolve(this.values!);
        }
    }

    private resolvedCnt = 0;
    private rejectedCnt = 0;
    private values: Array<T> = new Array<T>();
    private resolve: (value: Array<T>) => void;
    private reject: (error: Error) => void;
}

class AllSettledPromiseConcurrency<T> extends PromiseConcurrencyBase<T> {
    constructor(resolve: (value: PromiseSettledResult<T>[]) => void, reject: (error: Error) => void) {
        this.resolve = resolve;
        this.reject = reject;
    }

    allSettled(promises: FixedArray<T|PromiseLike<T>|undefined>): void {
        this.values = new PromiseSettledResultBase[promises.length];
        if (promises.length == 0) {
            this.resolve(this.getValues());
            return;
        }
        for (let i = 0; i < promises.length; ++i) {
            let idx = i;
            this.toPromiseLike(promises[idx]!).then<void, void>((value: T): void => {
                this.resolveImpl(value as T, idx);
            }, (error: Error): void => {
                this.rejectImpl(error as Error, idx);
            });
        }
    }

    private resolveImpl(value: T, idx: int): void {
        this.values![idx] = new PromiseFulfilledResult<T>(value);
        ++this.resolvedCnt;
        if (this.resolvedCnt == this.values!.length) {
            this.resolve(this.getValues());
        }
    }

    private rejectImpl(error: Error, idx: int): void {
        this.values![idx] = new PromiseRejectedResult(error);
        ++this.resolvedCnt;
        if (this.resolvedCnt == this.values!.length) {
            this.resolve(this.getValues());
        }
    }

    private getValues(): PromiseSettledResult<T>[] {
        return (this.values as Object) as PromiseSettledResult<T>[];
    }

    private resolvedCnt = 0;
    private rejectedCnt = 0;
    private values?: PromiseSettledResultBase[];
    private resolve: (value: PromiseSettledResult<T>[]) => void;
    private reject: (error: Error) => void;
}

class AnyPromiseConcurrency<T> extends PromiseConcurrencyBase<T> {
    constructor(resolve: (value: T) => void, reject: (error: Error) => void) {
        this.resolve = resolve;
        this.reject = reject;
    }

    any(promises: FixedArray<T|PromiseLike<T>|undefined>): void {
        this.errors = new Error[promises.length];
        if (promises.length == 0) {
            this.reject(new AggregateError(this.errors!, "All promises are rejected"));
            return;
        }
        for (let i = 0; i < promises.length; ++i) {
            let idx = i;
            try {
                this.toPromiseLike(promises[idx]!).then<void, void>((value: T): void => {
                    this.resolveImpl(value as T);
                }, (error: Error): void => {
                    this.rejectImpl(error as Error, idx);
                });
            } catch (e: Error) {
                this.rejectImpl(e, i);
            }
        }
    }

    private resolveImpl(value: T) {
        ++this.resolvedCnt;
        if (this.resolvedCnt == 1) {
            this.resolve(value);
        }
    }

    private rejectImpl(error: Error, idx: int) {
        ++this.rejectedCnt;
        if (error == undefined || error instanceof Error) {
            this.errors![idx] = error as Error;
        } else {
            this.errors![idx] = new Error(error!.toString());
        }
        if (this.rejectedCnt == this.errors!.length) {
            this.reject(new AggregateError(this.errors!, "All promises are rejected"));
        }
    }

    private resolvedCnt = 0;
    private rejectedCnt = 0;
    private errors?: Error[];
    private resolve: (value: T) => void;
    private reject: (error: Error) => void;
}

class RacePromiseConcurrency<T> extends PromiseConcurrencyBase<T> {
    constructor(resolve: (value: T) => void, reject: (error: Error) => void) {
        this.resolve = resolve;
        this.reject = reject;
    }

    race(promises: FixedArray<T|PromiseLike<T>|undefined>): void {
        for (let i = 0; i < promises.length; ++i) {
            let idx = i;
            this.toPromiseLike(promises[idx]!).then<void, void>((value: T): void => {
                this.resolveImpl(value as T);
            }, (error: Error): void => {
                this.rejectImpl(error as Error);
            });
        }
    }

    private resolveImpl(value: T) {
        if (!this.settled) {
            this.resolve(value)
            this.settled = true;
        }
    }

    private rejectImpl(error: Error) {
        if (!this.settled) {
            this.reject(error);
            this.settled = true;
        }
    }

    private settled = false;
    private resolve: (value: T) => void;
    private reject: (error: Error) => void;
}

export type NullablePromise<out T> = Promise<T> | undefined;
