..
    Copyright (c) 2021-2026 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


.. _Synchronization:

同步
********************************************************************************

.. meta:
    frontend_status: Done

|LANG| 及其标准库中提供的同步机制，用于满足以下需求：为异步或并行执行的 |C_JOBS| 所属代码施加特定执行顺序。
这种需求源于两个事实：

- 第一，|LANG| 中所有数据默认由所有 |C_WORKERS| 上的所有 |C_JOBS| 共享；如果同一数据被并发访问，则可能导致数据竞争；
- 第二，某些代码序列希望其所操作的数据仅由当前 |C_JOB| 独占访问。如果该 |C_JOB| 是一个 |C_CORO|，
  并且它在该代码序列中途挂起，那么即使另一个 |C_JOB| 运行在同一 |C_WORKER| 上，也可能破坏这种预期的独占性。

|LANG| 提供的同步手段包括：

- :ref:`AsyncLock`：一种“融合式”异步加锁 API，允许所提供的回调安全地操作某些数据；
- :ref:`AsyncMutex`、:ref:`AsyncRWLock`、:ref:`AsyncCondVar`：一种“解耦式”异步加锁 API，
  提供传统 ``lock()`` / ``unlock()`` 操作的异步版本，以及异步条件变量等价物；
- :ref:`Atomics`：原子操作支持，也可用于构建高效的无锁结构；
- :ref:`Other synchronization`：其他多种补充性的标准库类和方法。

|

.. _AsyncLock:

异步锁
================================================================================

.. meta:
    frontend_status: Done

异步锁（``AsyncLock`` 类）允许保护某些共享数据，例如对象状态的一部分，免受并发访问影响。
它适用于这样一种场景：访问受保护状态的代码序列可以方便地被隔离为一个独立的函数对象
（函数、方法或 lambda）。

.. code-block:: typescript
   :linenos:

   // a shared (e.g. global) data that we would like to protect
   class SharedState {
     value: string = "nothing"
   }
   let whatIsInTheBag: SharedState = new SharedState

   // a function that reads and modifies the shared data
   function checkAndModify(data: SharedState, expected: string, updated: string) {
     if (data.value != expected) {
       // data race!
       console.log("race!")
     }
     data.value = updated
   }

   // a suspension point emulator
   async function delay() {
     return new Promise<void>((res, rej) => {
       setTimeout(res, 1, undefined)
     })
   }

   // create a lock somewhere, for example as a global variable
   let lock = new AsyncLock()

   async function f() {
    // request an operation under the specified lock
    let p1 = lock.lockAsync<void>(async () => {
        // once the request can be satisfied, this lambda will run on the same
        // worker thread with the lock acquired

        // execute a modification sequence on the protected data:
        // nothing -> paraglider -> nothing

        // nothing -> paraglider
        checkAndModify(whatIsInTheBag, "nothing", "paraglider")

        // a sample suspension point that simulates a real life situation when
        // the modification sequence gets paused and another async function on
        // the same worker thread gets control
        await delay()

        // continue with the modification
        // paraglider -> nothing
        checkAndModify(whatIsInTheBag, "paraglider", "nothing")
    }, AsyncLockMode.EXCLUSIVE)

    // request another operation under the same lock: it will be executed not
    // earlier than the lock can be acquired
    let p2 = lock.lockAsync<void>(async () => {
        // another asynchronous modification sequence that suspends inbetween:
        // nothing -> apple -> nothing

        checkAndModify(whatIsInTheBag, "nothing", "apple")
        await delay()
        checkAndModify(whatIsInTheBag, "apple", "nothing")
    }) // AsyncLockMode.EXCLUSIVE is the default, so it can be skipped

    // wait for both operations to complete
    await p1
    await p2

    // Since both sequences have suspension points within them, they were
    // executed in an interleaved manner if there would no locks, which would
    // cause a data race and thus an error.
    // However, the lock that we used for synchronization prevents this
    // situation and each modification sequence executes as a critical section,
    // which leads to the correct result.
   }

   function main() {
     f()
   }

开发者可为给定 ``AsyncLock`` 请求两种访问独占级别之一：独占访问或共享访问。两者区别如下：

- 如果请求独占访问（默认行为），那么在请求者的回调执行完成之前，针对同一 ``AsyncLock`` 实例的其他任何回调执行请求都不会得到满足；
- 如果请求共享访问，那么在该锁下的其他共享访问请求可以并发得到满足，但所有要求独占访问的请求都必须等待轮到自己。

在 ``AsyncLock`` 下请求回调执行时，无论来自同一个还是不同的 |C_JOBS|，都可以并发安全地发起。

``AsyncLock`` API 提供了取消现有回调执行请求以及查询现有锁状态的方式。此外，它还支持为已发出的加锁请求设置超时以限制等待时间，并给出潜在死锁的提示。

更多信息请参见标准库文档。


.. _AsyncMutex:

异步互斥量
================================================================================

.. meta:
    frontend_status: Done

异步互斥量（``AsyncMutex``）允许保护某些共享数据，例如对象状态的一部分，免受并发访问影响。
它适用于以下场景：

- 开发者希望使用条件变量（:ref:`AsyncCondVar`）；
- 访问受保护状态的代码序列不易方便地隔离为独立的函数对象（函数、方法或 lambda），因此需要解耦的 ``lock()`` 与 ``unlock()`` 操作。

.. code-block:: typescript
   :linenos:

   // a shared (e.g. global) data that we would like to protect
   class SharedState {
     value: string = "nothing"
   }
   let whatIsInTheBag: SharedState = new SharedState

   // a function that reads and modifies the shared data
   function checkAndModify(data: SharedState, expected: string, updated: string) {
     if (data.value != expected) {
       // data race!
       console.log("race!")
     }
     data.value = updated
   }

   // a suspension point emulator
   async function delay() {
     return new Promise<void>((res, rej) => {
       setTimeout(res, 1, undefined)
     })
   }

   // create a lock somewhere, for example as a global variable
   let lock = new AsyncMutex()

   async function f() {
    // here we execute a modification sequence on the protected data under the
    // lock: nothing -> paraglider -> nothing

    // the await is mandatory! the promise returned by the lock() method will
    // get resolved not earlier than the lock is successfullly acquired
    await lock.lock();
    // the code between lock() and unlock() acts like a critical section:
    // no other job is able to acquire the "lock" till the "unlock()" is called

    // nothing -> paraglider
    checkAndModify(whatIsInTheBag, "nothing", "paraglider")

    // a sample suspension point that simulates a real life situation when
    // the modification sequence gets paused and another async function on
    // the same worker thread gets control
    await delay()

    // continue with the modification
    // paraglider -> nothing
    checkAndModify(whatIsInTheBag, "paraglider", "nothing")

    // end of the critical section
    lock.unlock()
   }

   async function g() {
    // another asynchronous modification sequence that suspends inbetween:
    // nothing -> apple -> nothing
    await lock.lock()
    // start of the critical section

    checkAndModify(whatIsInTheBag, "nothing", "apple")
    await delay()
    checkAndModify(whatIsInTheBag, "apple", "nothing")

    // end of the critical section
    lock.unlock()
   }

   function main() {
    // Call both functions consequently without any waits.
    // Since both functions have suspension points within them, they were
    // executed in an interleaved manner if there would no locks, which would
    // cause a data race and thus an error.
    // However, the mutex we used for synchronization prevents this
    // situation and each modification sequence executes as a critical section,
    // which leads to the correct result.
     f()
     g()
   }

``AsyncMutex`` 可安全用于运行在同一 |C_WORKER| 上的 |C_JOBS|，也可安全用于运行在不同 |C_WORKERS| 上的 |C_JOBS|。

避免重复加锁（即当前 |C_JOB| 已持有该锁实例时，又对该锁实例调用 `lock()` 方法）以及避免死锁，是开发者自身的责任。
更多信息请参见标准库文档。

.. _AsyncRWLock:

异步读写锁
================================================================================

.. meta:
    frontend_status: Done

异步读写锁（``AsyncRWLock``）允许保护某些共享数据，例如对象状态的一部分，免受并发访问影响。
它适用于以下两个条件同时成立的场景：

- 访问受保护状态的代码序列不易方便地隔离为独立的函数对象（函数、方法或 lambda），因此需要解耦的 ``lock()`` 与 ``unlock()`` 操作；
- 对共享状态的访问，需要在一组能够并发安全访问该数据的实体（“读者”）与任何要求对该数据独占访问的实体（“写者”）之间保持互斥。

更多信息请参见标准库文档。

.. _AsyncCondVar:

异步条件变量
================================================================================

.. meta:
    frontend_status: Done

异步条件变量（``AsyncCondVar``）适用于这样的场景：某些共享数据在一个 |C_JOB| 中被用作一系列动作的条件，
同时在另一个 |C_JOB| 中被并发修改。

使用 ``AsyncCondVar`` 需要 :ref:`AsyncMutex`：

.. code-block:: typescript
   :linenos:

   // create mutex and condition variable somewhere, e.g. in the global scope
   let m = new AsyncMutex();
   let cv = new AsyncCondVar();
   // the shared data, which is used as a condition
   let flag = false;

   async function f() {
    // the notification sequence (in job A):
    // lock the mutex
    await m.lock()
    // start of the critical section

    // update the condition
    flag = true
    // notify the waiter(s):
    // the API requires that the same mutex is to be provided here
    cv.notifyOne(m)

    // end of the critical section: unlock the mutex
    m.unlock()
   }

   async function g() {
    // the wait-and-react sequence (in job B):
    // lock the same mutex that is used for condition update and notification
    await m.lock() // await is mandatory!
    // start of the critical section

    // check the shared condition
    while (flag == false) {
      // start waiting for the condition to change:
      // the API requires that the same mutex is to be provided here
      // wait() unlocks "m" and returns the Promise that is going to be resolved
      // once some other job calls notifyOne()/notifyAll()
      await cv.wait(m) // await is mandatory!

      // at this point, "m" is locked again, and the notification has been received
    }
    // here the condition is satisfied, and the mutex is locked:
    // any dependent actions (e.g. some state update) can happen here, and they
    // effectively happen in the same critical section with the verification of
    // the shared condition value

    // end of the critical section: unlock the mutex
    m.unlock()
   }

   function main() {
     f()
     g()
   }

更多信息请参见标准库文档。

|

.. _Atomics:

原子操作
================================================================================

.. meta:
    frontend_status: Done

|LANG| 标准库提供了一组支持原子操作的类。其预期用途是构建无锁数据结构与算法：从简单的 compare-and-swap 与自旋锁，到复杂容器均包括在内。

更多信息请参见标准库文档。

|

.. _Other synchronization:

其他实体与补充说明
================================================================================

.. meta:
    frontend_status: Done

|LANG| 标准库还提供多种附加类和 API，帮助开发者构建安全且高效的并发程序。这类类包括：

- 线程安全的并发容器；
- 操作 |C_WORKER| 本地数据的 API；
- 其他辅助工具。

更多信息请参见标准库文档。


.. index::
   synchronization

|
