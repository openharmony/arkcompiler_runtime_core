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

.. _Coroutines (Experimental):

Coroutines (Experimental)
*************************

.. meta:
    frontend_status: Done

A function or a lambda can be a *coroutine*. |LANG| supports *basic coroutines*
and *structured coroutines*.
*Basic coroutines* are used to create and launch a coroutine. The result is then
to be awaited. Details are provided in :ref:`Standard Library`.

.. index::
   function
   lambda
   structured coroutine
   basic coroutine
   coroutine

|

.. _Domains of Coroutines:

Domains of Coroutines
=====================

The Task domain defines capabilities of coroutines that are running on it.

- Main: The domain on which the main coroutine runs (the main JS coroutine);
- EA: Each EAcoroutine has a specific domain on which the coroutine runs
  exclusively;
- General: The domain on which launched coroutines, taskpool APIs, and native
  tasks (``async_work``) run.

Some domains are allowed to use cross-language interop (see the documentation
on cross-language interoperability for detail), while others are not:

.. table::
  :widths: auto

  ========= =========
   Domain    Interop
  ========= =========
   Main        yes
   EA          yes
   General     no
  ========= =========

|

.. _Root Coroutine:

Root Coroutine
==============

All coroutines in an application forms a forest 
where each coroutine is a node of some tree.
The topmost coroutine of a tree is called *root coroutine*.

Each corroutine except *root corutine* has a parent, i.e., a coroutine in
which the coroutine is created.

In general, the following root coroutines exist:

- Main Acoroutine, and
- EAcoroutine.

|

.. _Lifetime of Coroutines:

Lifetime of Coroutines
----------------------

By default the lifetime of launched coroutines is not limited and we will wait for their finish at the top-level parent exit. The lifetime of Main coroutine is depends on lifetime of all EAcoroutines, i.e., at the end if Main coroutine will wait when all EAcoroutines will be finished.

|

.. _Lifetime of async Functions:

Lifetime of ``async`` Functions
-------------------------------

For async functions lifetime is limited to the lifetime of root coroutine for coroutine from which this async function created. When root coroutine is finished - it is no guarantee that any code will be executed in async function.

|

.. _Threads Structure and Types of Coroutines:

Threads Structure and Types of Coroutines
-----------------------------------------

In |LANG| VM we have M:N scheduling, i.e., for M coroutines execution N OS-threads are used. The only special thread is `main Thread` and it is always exist. At start we have main Jcoroutine (it is the default main context for execution of JS code) and main Acoroutine(it is default main context for execution of |LANG| code) running on it.

We have different types of coroutines in |LANG|:

_`Jcoroutine` - ``async`` |JS| function called via interop from the Main or EA domain


_`Acoroutine` - M:N coroutine which can be scheduled to any available thread (except the thread with the main Acoroutine)

_`AJcoroutine` - |LANG| ``async`` function (J - means that probably this code was just migrated *as is* from |JS|/|TS| code, and we have no guarantee that it is safe to apply M:N scheduling for such functions),

_`EAcoroutine` - coroutine which occupy some thread explicitly for itself and for all children (recursively)


Coroutine affinity is described in the table below:

.. table::
  :widths: 20 10 15 15 40

  ================ ================== ================= =========== ===========================================================
  Coroutine Type   Domain             Affinity with     Context     How Coroutine is Created
  ================ ================== ================= =========== ===========================================================
  Jcoroutine       Main, EA           root coroutine    |JS| / |TS| interop |LANG| -> |JS| / |TS|, or directly from |JS| / |TS|
  Main Acoroutine  Main               main thread       |LANG|      interop |JS| / |TS| -> |LANG|
  Acoroutine       Main, General, EA  \-                |LANG|      from main Acoroutine via launch
  AJcoroutine      Main, General, EA  parent coroutine  |LANG|      from Acoroutine(any) via call of ``async`` function
  ================ ================== ================= =========== ===========================================================

.. note::
    Affinity with - it means which policy is used for forced coroutine migration, for example if it is `parent coroutine` - it means it is always migrate on the thread with parent coroutine



The reason to have different types of coroutines is to isolate code which is not suitable to use in concurrent mode and run it with some restrictions.

For the Acoroutine and AJcoroutine developer is responsible for the checking that the code is ok to run on the thread different from the main thread or EA thread.


The EAcoroutine is essentially special type of coroutine which is running on isolated thread exclusively with its children coroutines.

|

.. _Interop and Concurrency:

Interop and Concurrency
-----------------------

|

.. _Concurrency Cross-Language Interop:

Concurrency: Cross-Language Interop
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When we have Dynamic Object (i.e., static object contains reference to the object obtained from JSVM), we have some limitations for it:

#. we can use such objects only in the Main and EA domains
#. we can't transfer such objects to the coroutines if we are operating in M:N mode
#. we can't transfer such objects to the TaskPool

If it is possible we need to locate issue with incorrect usage/transfer at Frontend level. If not, we should check at runtime and throw exception.

.. todo::
    Define type of exception thrown when passing non sendable object to the Taskpool or to coroutine in M:N mode

|

.. _Concurrency Native Interop:

Concurrency: Native Interop
~~~~~~~~~~~~~~~~~~~~~~~~~~~

In |LANG| we can have calls to native code. When we have native call the VM cannot control what is happening in it, because the VM is not responsible for execution of native code.
It means, that we can't interrupt the coroutine execution for rescheduling. The consequences of it are different, for example if it will happen in the coroutine at the main thread, it will block UI thread and OS could kill application.
