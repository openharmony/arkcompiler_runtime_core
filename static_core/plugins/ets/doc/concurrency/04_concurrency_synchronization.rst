..
    Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

Synchronization
***************

.. meta:
    frontend_status: Done

The synchronization mechanisms that exist in |LANG| and its standard library
address the need for imposing certain order on the execution of the code that
belongs to the |C_JOBS| being executed asynchronously or in parallel. Such need
originates from the two facts:

- firstly, all the data in |LANG| are by default shared between all |C_JOBS| on
  all |C_WORKERS|, which may cause data races in case when the same data is
  accessed concurrently;
- secondly, certain code sequences expect the data they operate on to be
  accessed exclusively by their |C_JOB|. If this |C_JOB| is a |C_CORO| and it
  suspends its execution inbetween of such code sequence, then another |C_JOB|
  can violate the expected exclusivity even in case when it runs on the same
  |C_WORKER|.

The means of synchronization that |LANG| provides are:

- :ref:`AsyncLock`: the "fused" asynchronous locking API, which allows the
  provided callback to safe operate on some data;
- :ref:`AsyncMutex`, :ref:`AsyncRWLock`, :ref:`AsyncCondVar`: the "decoupled"
  asynchronous locking API, which provides the asynchronous version of
  traditional decoupled ``lock()`` / ``unlock()`` operations and also an
  asynchronous condition variable equivalent;
- :ref:`Atomics`: the atomic operations support, which also allows for building
  efficient lock-free structures
- :ref:`Other synchronization`: a variety of other supplemental standard library
  classes and methods
  
|

.. _AsyncLock:

Asynchronous lock
=================

.. meta:
    frontend_status: Done

The asynchronous lock (``AsyncLock`` class) allows to protect some shared data,
for example, a part of object state, from concurrent access. It is designed for
the use cases when the code sequence that accesses the protected state can be
conveniently isolated as a distinct function object (function, method or
lambda).

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

A developer can request one of two levels of access exclusivity to the given
``AsyncLock``: exclusive or shared. The difference is as follows:

- if an exclusive access is requested (default behaviour), then no other request
  for callback execution under the same ``AsyncLock`` instance will be satisfied
  until the requester's callback is finished;
- if a shared access is requested then any other request for the callback
  execution under this lock can be concurrently satisfied, but all requests that
  demand exclusive access will wait their turn

  The callback execution under an ``AsyncLock`` can be safely requested
  concurrently both by the same and different |C_JOBS|.

``AsyncLock`` API provides a way to abort an existing request for callback
execution and to query the status of the existing locks. Additionally it
provides a way to limit the waiting time for an issued lock acquire request with
a timeout and also gives hints about the potential deadlocks.

Please refer to the standard library documentation to find out more information.


.. _AsyncMutex:

Asynchronous mutex
==================

.. meta:
    frontend_status: Done

The asynchronous mutex (``AsyncMutex``) allows to protect some shared data,
for example, a part of object state, from concurrent access. It is designed for
the following use cases:

- developer wants to use a condition variable (:ref:`AsyncCondVar`)
- the code sequence that accesses the protected state is hard to be conveniently
  isolated as a distinct function object (function, method or lambda), so the
  decoupled ``lock()`` and ``unlock()`` operations are required

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

The ``AsyncMutex`` can be safely used in both the |C_JOBS| that run on the same
|C_WORKER| and on different |C_WORKERS|.

The avoidance of double locking (happens if the `lock()` method is called from
the lock instance that is already acquired by the current |C_JOB|) and deadlocks
is the developer's responsibility. Please refer to the standard library
documentation to find out more information.

.. _AsyncRWLock:

Asynchronous read/write lock
============================

.. meta:
    frontend_status: Done

The asynchronous read/write lock (``AsyncRWLock``) allows to protect some shared
data, for example, a part of object state, from concurrent access. It is
designed for the use case when both of the following statements are true:

- the code sequence that accesses the protected state is hard to be conveniently
  isolated as a distinct function object (function, method or lambda), so the
  decoupled ``lock()`` and ``unlock()`` operations are required
- access to the shared state must be mutually exclusive between a group of
  entities that can safely access the data concurrently ("readers") and any
  other entity that requires exclusive access to the data ("writer"/"writers")

Please refer to the standard library documentation to find out more information.

.. _AsyncCondVar:

Asynchronous condition variable
===============================

.. meta:
    frontend_status: Done

The asynchronous condition variable (``AsyncCondVar``) is designed for the use
case when some shared data is used as a condition for a sequence of actions in
one |C_JOB| and is concurrently modified in another |C_JOB|.

The use of ``AsyncCondVar`` requires :ref:`AsyncMutex`:

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

Please refer to the standard library documentation to find out more information.

|

.. _Atomics:

Atomic operations
=================

.. meta:
    frontend_status: Done

|LANG| standard library provides a set of classes that support atomic
operations. The intended use cases for them are lock free data structures and
algorithms: from simple compare-and-swap and spinlocks to complex containers.

Please refer to the standard library documentation to find out more information.

|

.. _Other synchronization:

Additional entities and other notes
===================================

.. meta:
    frontend_status: Done

The |LANG| standard library provides various additional classes and APIs that
help developers to build safe and efficient concurrent programs. Such classes
include:

- thread safe concurrent containers
- APIs that operate on |C_WORKER|-local data
- other helpers

Please refer to the standard library documentation to find out more information
about them.


.. index::
   synchronization

|

