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

###########################
ArkTS: concurrency
###########################


************
Introduction
************

In modern world most of the hardware have multiple number of cores. To achieve maximum performance software should be able to use more than one core in some scenarious(for example multimedia processing, data nalysis, simulation, modeling, databases etc.). 

Besides that it is various number of asynchronous API in different level, so it is crucial to have support for this too.

**********************************
Overview of concurrency sub-system
**********************************

======================================
Overview of major concurrency features
======================================

In ArkTS we have API for the asynchronous programming(i.e. some tasks can be suspended and resumed later), and support for the coroutines which can run in parallel(implicitly or explicitly).
Also, since ArkTS coroutines share the memory, the developer should be aware about possible issues with that and use some functionality to guarantee thread safety. 

To support these features and to provide machinery to make trustworthy concurrent programs easily, ArkTS have these items:

1. support for asynchronous features (async/await/Promise)
2. support for coroutines in the standard library
3. support for structured concurrency in standard library (TaskPool API)
4. support for synchronization primitives and "thread"-safe containers in standard library

=====================
Domains of coroutines
=====================

The Task domain defines capabilities of coroutines that are running on it. 

- Main - on this domain we running main coroutine, main JS coroutine 
- EA - for each EAcoroutine we have such domain on which the EA coroutine is running exclusively
- General - launched coroutines, taskpool API and native tasks (async_work) are running in this domain

For some domains it is allowed to use cross-language interop(see documentation about cross-language interoperability for details), for some is not:

.. table:: coroutines interop
  :widths: auto

  ========= =========
   Domain    Interop
  ========= =========
   Main        yes
   EA          yes
   General     no
  ========= =========

==============
Root coroutine
==============

Each coroutine has a parent - the coroutine in which the coroutine was created.

Root coroutine - is the top-most parent for the coroutine. 

I.e. all coroutines in application can be represented as forest. Each coroutine is the node in some tree in this forest. And the root of this tree is the root coroutine for all coroutines in this tree.

In general we can have these root coroutines:

- main Acoroutine
- EAcoroutine

======================
Lifetime of coroutines
======================

By default the lifetime of launched coroutines is not limited and we will wait for their finish at the top-level parent exit. The lifetime of Main coroutine is depends on lifetime of all EAcoroutines, i.e. at the end if Main coroutine will wait when all EAcoroutines will be finished. 

===========================
Lifetime of async functions
===========================

For async functions lifetime is limited to the lifetime of root coroutine for coroutine from which this async function created. When root coroutine is finished - it is no guarantee that any code will be executed in async function. 

=========================================
Threads structure and types of coroutines
=========================================

In ArkTS VM we have M:N scheduling, i.e. for M coroutines execution N OS-threads are used. The only special thread is `main Thread` and it is always exist. At start we have main Jcoroutine(it is the default main context for execution of JS code) and main Acoroutine(it is default main context for execution of ArkTS code) running on it.  

We have different types of coroutines in ArkTS:

_`Jcoroutine` - async JS function called via interop from the Main or EA domain

_`Acoroutine` - M:N coroutine which can be scheduled to any available thread(except the thread with the main Acoroutine)

_`AJcoroutine` - ArkTS async function(J - means that probably this code was just migrated as it is from JS/TS code and we don't have guarantee that it is safe to apply M:N scheduling for such functions), 

_`EAcoroutine` - coroutine which occupy some thread explicitly for itself and for all children(recursively)

.. table:: coroutines affinity
  :widths: auto

  ============================== =================== ================== ========= =================================================
   coroutine type                       Domain        Affinity with      context   how coroutine created                           
  ============================== =================== ================== ========= =================================================
   Jcoroutine                     Main, EA            root coroutine      JS/TS    interop ArkTS->JS/TS, or directly from JS/TS    
   main Acoroutine                Main                main thread         ArkTS    interop JS/TS->ArkTS                            
   Acoroutine                     Main, General, EA   \-                  ArkTS    from main Acoroutine via launch                 
   AJcoroutine                    Main, General, EA   parent coroutine    ArkTS    from Acoroutine(any) via call of async function 
  ============================== =================== ================== ========= =================================================


.. note::
    Affinity with - it means which policy is used for forced coroutine migration, for example if it is `parent coroutine` - it means it is always migrate on the thread with parent coroutine

The reason to have different types of coroutines is to isolate code which is not suitable to use in concurrent mode and run it with some restrictions. 

For the Acoroutine and AJcoroutine developer is responsible for the checking that the code is ok to run on the thread different from the main thread or EA thread.  

The EAcoroutine is esentially special type of coroutine which is running on isolated thread exclusively with its children coroutines.

=======================
Interop and concurrency
=======================

-----------------------------------
Concurrency: cross language interop
-----------------------------------

When we have Dynamic Object (i.e. static object contains reference to the object obtained from JSVM), we have some limitations for it:

#. we can use such objects only in the Main and EA domains
#. we can't transfer such objects to the coroutines if we are operating in M:N mode
#. we can't transfer such objects to the TaskPool

If it is possible we need to locate issue with incorrect usage/transfer at Frontend level. If not, we should check at runtime and throw exception.

.. todo::
    Define type of exception thrown when passing non sendable object to the Taskpool or to coroutine in M:N mode

---------------------------
Concurrency: native interop
---------------------------

In ArkTS we can have calls to native code. When we have native call the VM cannot control what is happening in it, because the VM is not responsible for execution of native code.
It means, that we can't interrupt the coroutine execution for rescheduling. The consequences of it are different, for example if it will happen in the coroutine at the main thread, it will block UI thread and OS could kill application.9