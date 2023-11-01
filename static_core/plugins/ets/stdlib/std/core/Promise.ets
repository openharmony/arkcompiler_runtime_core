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

package std.core;

/**
 * Class represents a result of an asynchronous operation in the future.
 */
export final class Promise<T> {
    private constructor() {
    }

    constructor(callback: (resolve: (value: T) => void) => void throws) {
        try {
            callback((value: T): void => {
                this.resolveImpl(value);
            });
        } catch (error) {
            this.rejectImpl(error);
        }
    }

    constructor(callback: (resolve: (value: T) => void throws, reject: (error: Object|null) => void) => void throws) {
        try {
            callback((value: T): void => {
                this.resolveImpl(value);
            }, (error: Object|null): void => {
                this.rejectImpl(error)
            });
        } catch (error) {
            this.rejectImpl(error);
        }
    }

    then<U>(onFulfilled: () => U throws): Promise<U> {
        let promise = new Promise<U>();
        // TODO(audovichenko): remove thisPromise variable
        let thisPromise = this;
        let fn: () => void = (): void => {
            try {
                if (thisPromise.state == Promise.STATE_RESOLVED) {
                    promise.resolveImpl(onFulfilled());
                } else {
                    promise.rejectImpl(thisPromise.value);
                }
            } catch (error) {
                promise.rejectImpl(error);
            }
        }
        if (this.state != Promise.STATE_PENDING) {
            Promise.addToJobQueue(fn as Object);
        } else {
            this.thenQueue = this.pushToQueue(fn as Object, this.thenQueue);
        }
        return promise;
    }

    then<U>(onFulfilled: (value: T) => U throws): Promise<U> {
        let promise = new Promise<U>();
        // TODO(audovichenko): remove thisPromise variable
        let thisPromise = this;
        let fn: () => void = (): void => {
            try {
                if (thisPromise.state == Promise.STATE_RESOLVED) {
                    promise.resolveImpl(onFulfilled(thisPromise.value!));
                } else {
                    promise.rejectImpl(thisPromise.value!);
                }
            } catch (e) {
                promise.rejectImpl(e);
            }
        }
        if (this.state != Promise.STATE_PENDING) {
            Promise.addToJobQueue(fn as Object);
        } else {
            this.thenQueue = this.pushToQueue(fn as Object, this.thenQueue);
        }
        return promise;
    }

    then<U>(onFulfilled: (value: T) => U throws, onRejected: (error: Object|null) => U throws): Promise<U> {
        let promise = new Promise<U>();
        // TODO(audovichenko): remove thisPromise variable
        let thisPromise = this;
        let fn: () => void = (): void => {
            try {
                if (thisPromise.state == Promise.STATE_RESOLVED) {
                    promise.resolveImpl(onFulfilled(thisPromise.value!));
                } else {
                    promise.resolveImpl(onRejected(thisPromise.value!));
                }
            } catch (e) {
                promise.rejectImpl(e);
            }
        }
        if (this.state != Promise.STATE_PENDING) {
            Promise.addToJobQueue(fn as Object);
        } else {
            this.thenQueue = this.pushToQueue(fn as Object, this.thenQueue);
            this.catchQueue = this.pushToQueue(fn as Object, this.catchQueue);
        }
        return promise;
    }

    catch<U>(onRejected: (error: Object|null) => U throws): Promise<U> {
        let promise = new Promise<U>();
        // TODO(audovichenko): remove thisPromise variable
        let thisPromise = this;
        let fn: () => void = (): void => {
            try {
                let res: U = onRejected(thisPromise.value);
                promise.resolveImpl(res);
            } catch (e) {
                promise.rejectImpl(e);
            }
        }
        if (this.state == Promise.STATE_REJECTED) {
            Promise.addToJobQueue(fn as Object);
        } else if (this.state == Promise.STATE_PENDING) {
            this.catchQueue = this.pushToQueue(fn as Object, this.catchQueue);
        }
        return promise;
    }

    finally(onFinally: () => void throws): Promise<T> {
        let promise = new Promise<T>();
        // TODO(audovichenko): remove thisPromise variable
        let thisPromise = this;
        let fn: () => void = (): void => {
            try {
                onFinally();
                if (thisPromise.state == Promise.STATE_RESOLVED) {
                    promise.resolveImpl(thisPromise.value!);
                } else {
                    promise.rejectImpl(thisPromise.value!);
                }
            } catch (e) {
                promise.rejectImpl(e);
            }
        }
        if (this.state != Promise.STATE_PENDING) {
            Promise.addToJobQueue(fn as Object);
        } else {
            this.thenQueue = this.pushToQueue(fn as Object, this.thenQueue);
            this.catchQueue = this.pushToQueue(fn as Object, this.catchQueue);
        }
        return promise;
    }

    static resolve(): Promise<void> {
        let p = new Promise<void>();
        p.resolveImpl(Void);
        return p;
    }

    static resolve<U>(value: U): Promise<U> {
        let p = new Promise<U>();
        p.resolveImpl(value);
        return p;
    }

    static reject<U>(value: U): Promise<U> {
        let p = new Promise<U>();
        p.rejectImpl(value);
        return p;
    }

    // NOTE(audovichenko) Replace Promise<Object> by Promise<out Object> when it gets available
    static all(promises: Promise<Object>[]): Promise<Object> {
        let p = new Promise<Object>((resolve: (value: Object) => void, reject: (error: Object|null) => void): void => {
            // This temp object is needed because es2panda cannot change captured primitives
            new PromiseConcurrency(resolve, reject).all(promises);
        });
        return p;
    }

    static allSettled(promises: Promise<Object>[]): Promise<Object> {
        let p = new Promise<Object>((resolve: (value: Object) => void, reject: (error: Object|null) => void): void => {
            // This temp object is needed because es2panda cannot change captured primitives
            new PromiseConcurrency(resolve, reject).allSettled(promises);
        });
        return p;
    }

    static any(promises: Promise<Object>[]): Promise<Object> {
        let p = new Promise<Object>((resolve: (value: Object) => void, reject: (error: Object|null) => void): void => {
            // This temp object is needed because es2panda cannot change captured primitives
            new PromiseConcurrency(resolve, reject).any(promises);
        });
        return p;
    }

    static race(promises: Promise<Object>[]): Promise<Object> {
        let p = new Promise<Object>((resolve: (value: Object) => void, reject: (error: Object|null) => void): void => {
            // This temp object is needed because es2panda cannot change captured primitives
            new PromiseConcurrency(resolve, reject).race(promises);
        });
        return p;
    }

    private static pushToQueue(job: Object, queue: Object[] | null): Object[] {
        let oldLength = 0;
        if (queue != null) {
            oldLength = queue.length;
        }
        let newQueue = new Object[oldLength + 1];
        for (let i = 0; i < oldLength; ++i) {
            newQueue[i] = queue?.[i]!;
        }
        newQueue[oldLength] = job;
        return newQueue;
    }

    public native awaitResolution(): T;

    private native resolveImpl(value: T): void;
    private native rejectImpl(error: Object|null): void;
    private static native addToJobQueue(job: Object): void;

    private static STATE_PENDING = 0;
    private static STATE_RESOLVED = 1;
    private static STATE_REJECTED = 2;

    // Order of fields should be the following
    // 1. Reference fields
    // 2. Primitive fields in mostly size decreasing order
    // filling alignment holes
    private value: T | null = null;
    private thenQueue: Object[] | null = null;;
    private catchQueue: Object[] | null = null;
    private linkedPromise: Object | null = null;
    private eventPtr: long;
    private padding0: int;
    private state: int = Promise.STATE_PENDING;
}

class PromiseConcurrency {
    constructor(resolve: (value: Object) => void, reject: (error: Object|null) => void) {
        this.resolve = resolve;
        this.reject = reject;
    }

    all(promises: Promise<Object>[]): void {
        let values = new Object[promises.length];
        if (promises.length == 0) {
            this.resolve(values);
            return;
        }
        for (let i = 0; i < promises.length; ++i) {
            let idx = i;
            let promise = promises[idx];
            promise.then<Object>((value: Object): Object|null => {
                values[idx] = value;
                ++this.resolvedCnt;
                if (this.resolvedCnt == values.length) {
                    this.resolve(values as Object);
                }
                return null;
            }, (error: Object|null): Object|null => {
                this.reject(error);
                return null;
            });
        }
    }

    allSettled(promises: Promise<Object>[]): void {
        let values: (Object|null)[] = new Object[promises.length];
        if (promises.length == 0) {
            this.resolve(values);
            return;
        }
        for (let i = 0; i < promises.length; ++i) {
            let idx = i;
            let promise = promises[idx];
            promise.then<Object>((value: Object): Object|null => {
                values[idx] = value;
                ++this.resolvedCnt;
                if (this.resolvedCnt == values.length) {
                    this.resolve(values as Object);
                }
                return null;
            }, (error: Object|null): Object|null => {
                values[idx] = error;
                ++this.resolvedCnt;
                if (this.resolvedCnt == values.length) {
                    this.resolve(values as Object);
                }
                return null;
            });
        }
    }

    any(promises: Promise<Object>[]): void {
        let errors = new Error[promises.length];
        if (promises.length == 0) {
            this.reject(new AggregateError(errors, "All promises are rejected"));
            return;
        }
        for (let i = 0; i < promises.length; ++i) {
            let idx = i;
            let promise = promises[idx];
            promise.then<Object>((value: Object): Object|null => {
                ++this.resolvedCnt;
                if (this.resolvedCnt == 1) {
                    this.resolve(value);
                }
                return null;
            }, (error: Object|null): Object|null => {
                ++this.rejectedCnt;
                if (error == null || error instanceof Error) {
                    errors[idx] = error as Error;
                } else {
                    errors[idx] = new Error(error.toString());
                }
                if (this.rejectedCnt == errors.length) {
                    this.reject(new AggregateError(errors, "All promises are rejected"));
                }
                return null;
            });
        }
    }

    race(promises: Promise<Object>[]): void {
        for (let i = 0; i < promises.length; ++i) {
            let idx = i;
            let promise = promises[idx];
            promise.then<Object>((value: Object): Object|null => {
                this.resolve(value)
                return null;
            }, (error: Object|null): Object|null => {
                this.reject(error);
                return null;
            });
        }
    }

    private resolvedCnt = 0;
    private rejectedCnt = 0;
    private resolve: (value: Object) => void;
    private reject: (error: Object|null) => void;
}
