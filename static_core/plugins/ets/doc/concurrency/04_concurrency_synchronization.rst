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
- :ref:`Unsafe locks`: the unsafe synchronous locking primitives
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

   // create a lock somewhere
   let lock = new AsyncLock()

   async function f() {
    // request an operation under the lock
    let p: Promise<void> = lock.lockAsync<void>(() => {
        accessSomeData()
    })
    await p // if required, wait until the callback execution request completes
   }

A developer can request one of two levels of access exclusivity to the given
``AsyncLock``: exclusive or shared. The difference is as follows:

- if an exclusive access is requested, then no other request for callback
  execution under the same ``AsyncLock`` instance will be satisfied until the
  requester's callback is finished;
- if a shared access is requested then any other request for the callback
  execution under this lock can be concurrently satisfied, but all requests that
  demand exclusive access will wait their turn

  The callback execution under an ``AsyncLock`` can be safely requested
  concurrently both by the same and different |C_JOBS|.

.. code-block:: typescript
   :linenos:

   // create a lock somewhere
   let lock = new AsyncLock()

   async function f() {
    await lock.lockAsync<void>(() => {
        accessSomeData()
    }, AsyncLockMode.EXCLUSIVE /* can also be AsyncLockMode.SHARED */)
   }

``AsyncLock`` API provides a way to abort an existing request for callback
execution and to query the status of existing locks. Additionally it provides a
way to limit request waiting time with a timeout and gives hints about the
potential deadlocks. The detailed API description and the restrictions are in
the :ref:`API details and restrictions` section.


.. _AsyncMutex:

Asynchronous mutex
==================

.. meta:
    frontend_status: Done

The asynchronous mutex API (``AsyncMutex``) allows to protect some shared data,
for example, a part of object state, from concurrent access. It is designed for
the following use cases:

- developer wants to use a condition variable (:ref:`AsyncCondVar`)
- the code sequence that accesses the protected state is hard to be conveniently
  isolated as a distinct function object (function, method or lambda), so the
  decoupled ``lock()`` and ``unlock()`` operations are required

.. code-block:: typescript
   :linenos:

   // create a lock somewhere
   let lock = new AsyncMutex()

   async function f() {
    // acquire the lock
    await lock.lock()
    accessSomeData()
    lock.unlock()
   }

   async function g() {
    // acquire the lock
    await lock.lock()
    accessSomeData() // access the same data concurrently
    lock.unlock()
   }

The avoidance of double locking (happens if the `lock()` methods is called from
the lock instance that is already acquired by the current |C_JOB|) and deadlocks
is the developer's responsibility. The detailed API description and the
restrictions are in the :ref:`API details and restrictions` section.

.. _AsyncRWLock:

Asynchronous read/write lock
============================

.. meta:
    frontend_status: Done

The asynchronous read/write lock API (``AsyncRWLock``) allows to protect some
shared data, for example, a part of object state, from concurrent access. It is
designed for the use case when both of the following statements are true:

- the code sequence that accesses the protected state is hard to be conveniently
  isolated as a distinct function object (function, method or lambda), so the
  decoupled ``lock()`` and ``unlock()`` operations are required
- access to the shared state must be mutually exclusive between a group of
  entities that can safely access the data concurrently ("readers") and any
  other entity that requires exclusive access to the data ("writer"/"writers")

The detailed API description and the restrictions are in the :ref:`API details
and restrictions` section.

.. _AsyncCondVar:

Asynchronous condition variable
===============================

.. meta:
    frontend_status: Done

The asynchronous condition variable API (``AsyncCondVar``) is designed for the
use case when some shared data is used as a condition for a sequence of actions
in one |C_JOB| and is concurrently modified in another |C_JOB|.

The use of ``AsyncCondVar`` requires the use of ``AsyncMutex``:

.. code-block:: typescript
   :linenos:

   // create mutex and condition variable somewhere
   let m = new AsyncMutex();
   let cv = new AsyncCondVar();
   // the shared state
   let flag = false;

   async function f() { // called in the context of the first Job
    await m.lock() // lock the mutex
    flag = true // update the condition
    cv.notifyOne(m) // notify the waiter
    m.unlock() // unlock the mutex
   }

   async function g() { // called in the context of the second Job
    await m.lock() // lock the mutex
    while (flag == false) { // check the condition under the mutex
      await cv.wait(m) // atomically unlock the mutex and start waiting
      // if the promise returned by wait() is resolved,
      // it means that the cv has been notified and it
      // atomically locked the mutex and finished waiting
    }
    doSomething() 
    m.unlock()
   }

The detailed API description and the restrictions are in the :ref:`API details
and restrictions` section.

|

.. _Unsafe locks:

Unsafe synchronous locks
========================

.. meta:
    frontend_status: Done

For the specific use cases when developer needs synchronous wrappers over the OS
level synchronization primitives, |LANG| standard library offers the synchronous
locks API. They should be used with extreme caution, because the synchronous
wait blocks the whole OS thread that serves as a backend for an |LANG|
|C_WORKER|, which does not allow the |LANG| runtime environment to schedule
|C_JOBS| on this |C_WORKER| during the wait. In case of incorrect usage, this
can lead to deadlocks and performance penalties.

The detailed API description and the restrictions are in the :ref:`API details
and restrictions` section.

|

.. _Atomics:

Atomic operations
=================

.. meta:
    frontend_status: Done

|LANG| standard library provides a set of classes that support atomic
operations. The intended use cases for them are lock free data structures and
algorithms: from simple compare-and-swap and spinlocks to complex containers.

The detailed API description and the restrictions are in the :ref:`API details
and restrictions` section.

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

