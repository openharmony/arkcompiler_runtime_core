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

.. _Parallel execution:

Parallel execution
******************

.. meta:
    frontend_status: Done

The :term:`parallel execution` capability addresses the situation when
developer's code executes either CPU-intensive tasks that can take advantage of
utilizing multiple CPU cores or long tasks that can take advantage of running in
a separate OS thread of execution to avoid blocking.

For such cases, |LANG| provides a standard library level API that allows for
running code in parallel at function/method granularity (that is, for defining
|C_JOBS| that can run on different |C_WORKERS|), with the ability to specify
dependencies between |C_JOBS| and balance the load across the available CPU
cores and/or OS threads. 

This capability is orthogonal to the :ref:`Asynchronous execution` capability,
i.e. asynchronous functions can also be run in parallel, and this in general
does not affect the way they are suspended/resumed. The only difference is that
under certain conditions a |C_CORO| can change its |C_WORKER| upon resumption
(see :ref:`Scheduling rules`).

.. _Parallel execution API:

Parallel execution API
======================

.. meta:
    frontend_status: Done

|LANG| standard library provides the following sets of API for parallel
execution:

- :ref:`launch API`: the primary parallel execution API, simple and fast;
- :ref:`EAWorker API`: the API that allows for creation of |C_WORKERS| that are
  used exclusively by a |C_JOB| and its children;
- :ref:`Taskpool API`: the framework that offers structured concurrency
  capabilities: task grouping, dependencies and cancellation

|

.. _launch API:

``launch`` API
==============

The ``launch`` API is the primary parallel execution API. It launches the
provided lambda (synchronous or asynchronous) as a new |C_JOB|, by default
choosing the least busy |C_WORKER| to host it. If an asynchronous lambda is
provided as a body for this new |C_JOB|, and this asynchronous lambda has
suspension points, it can be rescheduled upon resumption to the |C_WORKER| that
is least busy at the time of resumption.

.. code-block:: typescript
   :linenos:

   async function f() {
    // ...

    // The full explicit form of launch.
    // Runs the provided lambda on the least busy worker thread and returns a 
    // promise that will get resolved once the lambda completes.
    let p1: Promise<Int> = launch<Int>(async () => {
        /* some long calculation here */
        await g()
    })
    let result1 = await p1 // can be safely awaited on the caller worker thread

    // Most of the details can be inferred from the context or omitted: 
    let p2 = launch { await g() }

    // ...
   }

   function h() {
       // launch is allowed in non-async functions, too
       launch { console.log("hello!") }
   }

The ``launch`` API allows to select the target |C_WORKER| for the new |C_JOB|
and to customize other launch parameters. The detailed API description and the
restrictions are in the :ref:`API details and restrictions` section.

.. index::
   launch

|

.. _EAWorker API:

EAWorker API
============

The EAWorker API is designed for the use case when developer's code requires a
dedicated |C_WORKER| to run on (for example, such use case is relevant for UI
frameworks). 

This API creates a |C_WORKER| for the *exclusive* use of an initial |C_JOB|.
That means, the initial |C_JOB| and all the |C_JOBS| spawned by it will stay on
this newly created |C_WORKER|, and no other |C_JOB| can be scheduled to this
|C_WORKER|.

The detailed API description and the restrictions are in the :ref:`API details
and restrictions` section.

.. index::
   EAWorker

|

.. _Taskpool API:

Taskpool API
============

The ``taskpool`` is the structured concurrency framework. It allows to create
new |C_JOBS|, specify dependencies between them, cancel the spawned |C_JOBS|,
combine them in groups and choose a complex execution order.

.. note::
    The |C_COROS| created by the taskpool API can not be rescheduled to another 
    |C_WORKER| upon resumption.

The detailed API description and other restrictions are in the :ref:`API details
and restrictions` section.

.. index::
   taskpool

|

