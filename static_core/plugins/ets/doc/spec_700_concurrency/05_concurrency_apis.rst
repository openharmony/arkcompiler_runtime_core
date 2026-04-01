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

.. _API details and restrictions:

API details and restrictions
****************************

.. meta:
    frontend_status: Done

This section describes the noteworthy details and the notable restrictions for
the concurrency APIs described in the previous sections.

.. _launch API details and restrictions:

``launch`` API details and restrictions
=======================================

.. meta:
    frontend_status: Done

The ``launch`` API allows for defining a set of |C_WORKERS| that a newly created
|C_JOB| can run on. This set is defined in terms of |C_WORKER| *domains* and
*groups*.

|C_WORKER| ID
  A unique number that is assigned to every |C_WORKER|, existing or newly
  created.

|C_WORKER| domain
  A named filtering criteria that defines some set of |C_WORKERS| that have
  something in common. A |C_WORKER| domain can contain different number of
  workers at different time. Notable domains include *main* and *exclusive*
  |C_WORKERS|. The exact list of available domains is provided in the standard
  library documentation.

|C_WORKER| group
  An immutable set of |C_WORKERS|.

The ``launch`` API allows to define a |C_WORKER| group in several ways, for
example by specifying the |C_WORKER| domain or the exact list of |C_WORKER| IDs.
Once defined, a |C_WORKER| group can be specified in the ``launch`` parameters,
so the newly created |C_JOB| will be assigned to the appropriate |C_WORKER| from
the provided |C_WORKER| group. Later on, if the scheduler decides to reschedule
this |C_JOB| to another |C_WORKER|, the new |C_WORKER| will be chosen from this
group, too.

.. note::
   Since the |C_WORKER| group is immutable, at some point the |C_WORKER| IDs it
   refers might become invalid. This happens because in some situations
   |C_WORKERS| can be created or deleted (e.g. the *exclusive* |C_WORKERS|),
   including the |C_WORKERS| that the group contains. In such case, the
   functions from `launch`` API will either safely ignore the invalid IDs, throw
   an error or return the appropriate return value. For the details, please
   refer to the standard library documentation.

|

.. _Using async API:

Using the asynchronous API
==========================

In certain cases, a call to an ``async`` function requires awaiting its result,
but the call site resides in the non-async function. In such cases, the caller
function should be converted to an asynchronous one, and in some cases this
chain of conversions has to be continued up to the program entry point. For this
case, |LANG| supports the ``async`` entry point function (see :ref:`Program Entry Point`).

.. note::
   Maybe, this section should be moved to the handbook.

|

.. _Promise API:

Promise class API
=================

There are some important restrictions that limit the correct usage of the
``Promise`` class.

A ``Promise`` class instance is safe to be awaited within a |C_JOB| on some
|C_WORKER| while being resolved or rejected from another |C_JOB| on the same or 
different |C_WORKER|.

A ``Promise`` class allows to register callbacks that are to be called upon
``Promise`` resolution and/or rejection. This is done by calling the ``.then()``
/ ``.catch()`` / ``.finally()`` methods of the ``Promise`` class. However, these
methods have the following usage restrictions:

- the registered callback will be called as a separate |C_JOB| on the same
  |C_WORKER| where it was registered
- if multiple callbacks are registered from the |C_JOBS| that reside on the same
  |C_WORKER|, the order of their execution matches the order of their
  registration
- if multiple callbacks are registered from the |C_JOBS| that reside on
  different |C_WORKERS|, the order of their execution is defined only within
  each |C_WORKER|, and no order is guaranteed between the resulting |C_JOBS|
  that reside on different |C_WORKERS|

Please refer to the standard library documentation to find out more information
about the ``Promise`` methods.

|

.. _Unhandled Rejected Promises:

Unhandled Rejected Promises
===========================

.. meta:
    frontend_status: Done

.. note::
   The semantics of unhandled rejections will be revisited later, once the
   design of |LANG| concurrency subsystem is complete.

A rejected ``Promise`` is considered unhandled if, at certain time, there is no
``await`` waiting for it and there are no callbacks registered for it with the
``.then()`` / ``.catch()`` methods.

This moment of time is defined separately on each |C_WORKER|, hence the
``Promise`` instance is considered an *unhandled rejection* only within a
context of some |C_WORKER|, while possibly being *handled* on other ones.

.. index::
   unhandled promise
   rejected promise
   unhandled rejection
   rejection handler
   call
   program completion

|

.. _Concurrency error handling:

Error handling policy
=====================

In general, all errors thrown in an |LANG| program should either have an ability
to be handled by the developer or considered uncaught, and initiate a program
termination sequence. This applies to any |C_JOB| on any |C_WORKER|.

A |C_JOB| in an |LANG| program can complete abnormally, i.e., can throw an error.
Since |C_JOBS| communicate their return values using ``Promise`` class
instances, in case of |C_JOB|'s abnormal completion the corresponding promise
gets rejected and the original error is not considered uncaught.

However, there can exist some cases when such rejection cannot be handled by the
developer, for example:

- when the thrower |C_JOB| was created by the runtime environment, and no
  promise can be awaited or handled with a ``.then()`` / ``.catch()`` callback
- when the *main* |C_JOB| throws an error

In such cases, the original error thrown by the |C_JOB| will be considered
uncaught.

|



