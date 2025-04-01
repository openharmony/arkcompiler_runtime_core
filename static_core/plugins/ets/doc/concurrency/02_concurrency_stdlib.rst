..
    Copyright (c) 2025 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

#########################
ArkTS Concurrency: stdlib
#########################

********
Overview
********

In general we have several directions for concurrency in ArkTS stdlib:

- interfaces for coroutines
- concurrency/async primitives required for the compatibility with JS/TS
- support for the OHOS-specific concurrency features(such as TaskPool, AsyncLock, etc.)
- new concurrent containers/concurrency primitives 

*************************
Interfaces for coroutines
*************************

.. _Concurrency Job:

===
Job
===

Job is the class which represent the job/task executing on the coroutine.

Job object represents the job which will be scheduled and executed. There is no guarantee that it will be executed on the same thread when it was created.

If the during execution job function throws error this error will be propagated to the `Await` method and rethrown. 

In general Job class have these public API:

.. code-block:: typescript

  class Job<T> {
    constructor(v: T) {
        this.val = v
    }

    // Awaits execution of the job.
    // On success return value, on fail the Error will be thrown 
    Await() : T {
      // ...
    }
  }


.. _Concurrency launch:

======
launch
======

For creation of the coroutine you should use standard library function ``launch`` with this signature(Note: final inteface is still will be defined, current is just to demonstrate semantic):

.. code-block:: typescript

  function launch<R, F extends CoroFun<never, R>>() {
    coroFun: F,
    ..args: (Object | null | undefined)[]
  ) : Job<R>


Where `coroFun` is either lambda, either static method or function.

The created coroutine will be scheduled on the one of the coroutine worker threads and can be rescheduled to another later.

***************************************
JS/TS compatible concurrency primitives
***************************************

.. _Concurrency Promise Class:

=======
Promise
=======

The Promise object is introduced for the support asynchronous API. It is the object which represents a proxy for the result of the asynchronous operation. The semantic of the Promise is similar to the semantic of Promise in JS/TS if it used in the context of one coroutine.

Promise object represents the values returned by call of async function.
It belongs to the core packages of the standard library, 
and thus it is imported by default and may be used
without any qualification.

Promise lifetime is not limited to the lifetime of the root coroutine when it was created.

In general promise not designed to be used concurrently simultaneously from multiple coroutines. But it is safe to do this:

- pass promise from one coroutine to another and do not use it anymore in original coroutine
- pass promise from one coroutine to another, use it in both coroutines but call `then` only in one coroutine
- pass promise from one coroutine to another, use it in both coroutines, call `then` in both coroutines, but user provide custom synchronization which guarantee that `then` will not be called simultaneously for this promise

The following methods are used as follows:

-  ``then`` takes two arguments (the first argument is the callback used if the
   promise is fulfilled, and the second if it is rejected), and returns
   ``Promise<U>``. If ``then`` called from same parent coroutine multiple times - the order of ``then`` is the same if it is called in JS/TS.
   The callback is called on the coroutine when ``then`` called, so if you pass promise from one coroutine to another
   and called ``then`` in both - they will be called in different coroutines and maybe concurrently, so developer should
   take care about possible data races.

.. index::
   class
   value
   launch
   argument
   callback
   package
   standard library
   method

..
        Promise<U>::then<U, E = never>(onFulfilled: ((value: T) => U|PromiseLike<U> throws)|undefined, onRejected: ((error: NullishType) => E|PromiseLike<E> throws)|undefined): Promise<U|E>

.. code-block:: typescript

        Promise<U>::then<U, E = never>(onFulfilled: ((value: T) => U|PromiseLike<U> throws)|undefined, onRejected: ((error: NullishType) => E|PromiseLike<E> throws)|undefined): Promise<U|E>

-  ``catch`` takes one argument(the callback called after promise is rejected) and returns ``Promise<U|T>``

.. code-block-meta:

.. code-block:: typescript

        Promise<U>::catch<U = never>(onRejected?: (error: NullishType) => U|PromiseLike<U> throws): Promise<T|U>

-  ``finally`` takes one argument (the callback called after ``promise`` is
   either fulfilled or rejected) and returns ``Promise<T>``.

.. index::
   alias
   callback
   call

.. code-block:: typescript

        finally(onFinally?: () => void throws): Promise<T> 

****************************
Concurrency extensions
****************************

Besides JS/TS compatible concurrency primitives, there are some extensions in ArkTS which introduce some additional concurrency functionality. 

========
TaskPool
========

TaskPool provides multi-threaded environments for applications. It helps to run sequence of tasks on the pool of threads. Also you shouldn't care about managing this pool: the TaskPool itself manage lifetime of threads in the pool, their number, etc.

TaskPool allows to reduce resource consumption and improve system performance.

-----------------------
TaskPool for JS context
-----------------------

TaskPool could be used in JS context and in the static context.

The unit of execution in TaskPool is concurrent function (function with @Concurrent decorator for JS/TS compatible mode, for M:N mode this decorator is optional).

We have some limitations for the TaskPool used in JS context:

* functions used as tasks in TaskPool should be defined with @Concurrent decorator
* it is not allowed to use closure variables in @Concurrent function 

---------------------------
TaskPool for static context
---------------------------

In static context we have same API as for JS context to the language syntax/semantic extent and we don't have any specific requirements for functions used as tasks
except one requirement which is applicable for all M:N coroutines: we shouldn't have interop in this function.

In general TaskPool provides structured concurrency features. I.e. it allow you to start some set of tasks, cancel task, wait for tasks, etc.

For detailed information about TaskPool please take a look at standard library documentation.

=========
AsyncLock
=========

For objects shared between different concurrent domains, it is crucial to have some machinery to provide some machinery for synchronization. One of the ways to guarantee thread-safe access to the object is Locking machinery. For this we introduce AsyncLock in ArkTS.

For languages with coroutines which are executing on the more than one CPU core we may need such synchronization primitive as Lock. But we can't use OS-level lock, since there are queue of coroutines waiting for execution on this core.

For this we need introduce special type of lock, which will not block the whole OS-level thread on such Lock.

.. uml:: os_based_lock_deadlock_seq.plantuml

In Java language we have `synchronized` methods for guarantee that only one thread executing such method. For ArkTS we can introduce special class `AsyncLock`, which have method `async` for running code 

.. code-block:: ts
    :linenos:

    class AsyncLock {
      async(lambda:any) {
        // acquire lock
        lambda();
        // release lock
      }
    }

The semantic of such lock should be something like this:

.. code-block:: c++
    :linenos:

    void Lock(ObjectHeader* obj) {
        while (1) {
            if (try_lock(obj) == SUCCESS) {
                return;
            }
            yield(); // suspend current coroutine
        }
    }
    
    bool try_lock(ObjectHeader* obj) {
        if (obj.SetState(LOCKED) == SUCCESS) {
          return SUCCESS;
        }
        return FAIL;
    }


For this it is enough to have special state in `ObjectHeader` and change it atomically. Or we can have just some atomic field `state`.

But `while (1)` can be optimized if we will have explicit scheduler for such tasks. For example we can group locked coroutines by lock object, and have queue for unlock events, when we process something from this queue, we can add next coroutine from this queue to the queue for scheduler. 

Example with `AsyncLock` usage:

.. code-block:: javascript
    :linenos:

    import {AsyncLock, SyncMode} from '@ohos.sync';

    // @sendable
    class Demo {
      count: number = 0
      lock: AsyncLock = new AsyncLock();
      async add() {
        this.lock.async(lock => {
        this.count++;
        })
      }
      async get() {
        this.lock.async(lock => {
        return this.count;
        })
      }
    }


For VMs without shared memory, however, the implementation of AsyncLock requires different approach. Since we can't share object, we can obtain lock object by name or id from different threads. And the lock object should be accessible from any VM instance. The same approach applicable for the VM with shared memory.



..
  /**
   * Information about all lock operations on the AsyncLock instance.
   *
   * @syscap SystemCapability.Utils.Lang
   * @crossplatform
   * @atomicservice
   * @since 12
   */
  class AsyncLockState {
    /**
     * Array of lock operations which held the lock.
     *
     * @syscap SystemCapability.Utils.Lang
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    held: AsyncLockInfo[];
    /**
     * Array of lock operations waiting for the lock.
     *
     * @syscap SystemCapability.Utils.Lang
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    pending: AsyncLockInfo[];
  }
  /**
   * Information about a lock and a lock operation.
   *
   * @syscap SystemCapability.Utils.Lang
   * @crossplatform
   * @atomicservice
   * @since 12
   */
  class AsyncLockInfo {
    /**
     * Identifier of the lock if the lock is anonymous. For named locks this field is undefined
     *
     * @syscap SystemCapability.Utils.Lang
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    id?: number;
    /**
     * Name of the named lock. For anonymous locks this field is undefined.
     *
     * @syscap SystemCapability.Utils.Lang
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    name?: string;
    /**
     * Lock operation's mode.
     *
     * @syscap SystemCapability.Utils.Lang
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    mode: AsyncLockMode;
    /**
     * lockAsync caller thread's identifier.
     *
     * @syscap SystemCapability.Utils.Lang
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    threadId: number;
  }
  /**
   * Object to abort an async operation.
   *
   * @syscap SystemCapability.Utils.Lang
   * @crossplatform
   * @atomicservice
   * @since 12
   */
  class AbortSignal<T> {
    /**
     * Set to true to abort an operation
     *
     * @syscap SystemCapability.Utils.Lang
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    aborted: boolean;

    /**
     * Reason of the abort. This value will be used to reject the promise returned from lockAsync.
     *
     * @syscap SystemCapability.Utils.Lang
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    reason: T
  }

===========================
AsyncLock Deadlock Detector
===========================

It is possible that the developer make a mistake and create code which lead to the deadlock situation while using AsyncLock. For this it is possible to specify the maximum time which we expect is enough for successful Lock acquirence. In case if we reach the limit - the provided by developer callback will be called. 
