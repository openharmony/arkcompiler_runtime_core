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

    constructor(callback: (resolve: (value: T) => void throws, reject: (error: Object) => void) => void throws) {
        try {
            callback((value: T): void => {
                this.resolveImpl(value);
            }, (error: Object): void => {
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
                    promise.resolveImpl(onFulfilled(thisPromise.value));
                } else {
                    promise.rejectImpl(thisPromise.value);
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
                    promise.resolveImpl(onFulfilled(thisPromise.value));
                } else {
                    promise.rejectImpl(onRejected(thisPromise.value));
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
                    promise.resolveImpl(thisPromise.value);
                } else {
                    promise.rejectImpl(thisPromise.value);
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

    private static pushToQueue(job: Object, queue: Object[]): Object[] {
        let oldLength = 0;
        if (queue != null) {
            oldLength = queue.length;
        }
        let newQueue = new Object[oldLength + 1];
        for (let i = 0; i < oldLength; ++i) {
            newQueue[i] = queue[i];
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
    private value: T;
    private thenQueue: Object[];
    private catchQueue: Object[];
    private linkedPromise: Object;
    private eventPtr: long;
    private padding0: int;
    private state: int = Promise.STATE_PENDING;

}
