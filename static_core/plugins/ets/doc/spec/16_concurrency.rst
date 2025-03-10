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


.. _Concurrency:

###########
Concurrency
###########

|

.. _Introductory Note:

*****************
Introductory Note
*****************

Most modern hardware has multiple cores. To achieve maximum performance, the
software must be capable of using more than one core in some scenarios (e.g.,
multimedia processing, data analysis, simulation, modelling, databases etc.).

Providing support to a number of asynchronous APIs at different levels
is also crucial.

|

.. _Concurrency Subsystem Overview:

******************************
Concurrency Subsystem Overview
******************************

|

.. _Major Concurrency Features:

Major Concurrency Features
==========================

|LANG| has APIs for asynchronous programming that enables tasks to be suspended
and resumed later, and supports coroutines that can run in parallel (implicitly
or explicitly). Since the |LANG| coroutines share memory, a developer must be
aware about the possible associated issues, and use appropriate functionality
to guarantee thread safety.

|LANG| enables both asynchronous programming and parallel-run coroutines, and
provides machinery for trustworthy concurrent programs by providing the
following:

1. Asynchronous features ``async`` / ``await`` / ``Promise``;
2. Coroutines (experimental) in :ref:`Standard Library`;
3. Structured concurrency in :ref:`Standard Library` (TaskPool API);
4. Synchronization primitives and "thread"-safe containers in
   :ref:`Standard Library`.


|

.. _Async Functions and Methods:

****************
Asynchronous API
****************

.. meta:
    frontend_status: Done


.. _Async Functions:

``Async`` Functions
===================

.. meta:
    frontend_status: Done

``Async`` functions are implicit coroutines that can be called as regular
functions. ``Async`` functions cannot be called in static initializers,
including top-scope. ``Async`` functions can be neither ``abstract`` nor
``native``.

The return type of an ``async`` function must be ``Promise<T>``. It is
a library type discussed in detail in the |LANG| Concurrency Specification.

The returning values of both type ``Promise<T>`` and type ``T`` are allowed
inside the ``async`` function body (see :ref:`Return Type Inference`).

Using return statement without an expression is allowed if the return type
is ``Promise<void>``.
*No-argument* return statement can be added implicitly as the last statement
of the function body if there is no explicit return statement in a function
with the return ``Promise<void>``.

**Note**. Using type ``Promise<void>`` is not recommended as this type is
supported for the sake of backward |TS| compatibility only.

.. index::
   function async
   coroutine
   return type
   function body
   backward compatibility
   annotation
   no-argument return statement
   async function
   return statement
   compatibility

|

.. _Async Lambdas:

``Async`` Lambdas
=================

.. meta:
    frontend_status: Done

A lamdba with the modifier ``async`` (see :ref:`Lambda Expressions`)
is an implicit coroutine that can be called as a regular lambda.

``Async`` lambdas follow the same rules as :ref:`Async Functions`.

.. index::
   async lambda

|

.. _Concurrency Async Methods:

``Async`` Methods
=================

.. meta:
    frontend_status: Done

A class method with the modifier ``async`` (see :ref:`Method Declarations`)
is an implicit coroutine that can be called as a regular method.

``Async`` methods follow the same rules as :ref:`Async Functions`.

.. index::
   async method

|

.. _await:

await
=====

``await`` is used to wait for :ref:`Promise<Concurrency Promise Class>`

If ``Promise`` not resolved, then the current coroutine is suspended until
it is resolved.

If :ref:'Promise<Concurrency Promise Class>' is rejected, then the reason of
the rejection is thrown.

Using ``await`` outside of :ref:`async functions<async functions>` is forbidden.

|

.. _Concurrency Promise Class:

Promise
=======

The ``Promise object`` is introduced to support asynchronous API. It is the
object that represents a proxy for the result of an asynchronous operation. The
semantics of ``Promise`` is similar to the semantics of ``Promise`` in |JS|/|TS|
if it is used in the context of a single coroutine.

``Promise object`` represents the values returned by the call of an ``async``
function. ``Promise object`` belongs to core packages of the
:ref:`Standard Library`. Thus it is imported by default, and can be used
without any qualification.

The ``Promise`` lifetime is not limited to the lifetime of the root coroutine
as it is created.

``Promise`` is not in general designed to be used concurrently and
simultaneously from multiple coroutines. However, it is safe to do
the following:

- Pass ``Promise`` from one coroutine to another, and avoid using it again in
  the original coroutine.
- Pass ``Promise`` from one coroutine to another, use it in both coroutines,
  and call ``then`` only in one coroutine.
- Pass ``Promise`` from one coroutine to another, use it in both coroutines,
  and call ``then`` in both coroutines. The user is to provide custom
  synchronization to guarantee that ``then`` is not called simultaneously
  for this ``Promise``.

The methods are used as follows:

-  ``then`` takes two arguments. The first argument is the callback used if the
   promise is fulfilled. The second argument is used if it is rejected, and
   returns ``Promise<U>``.

-  If ``then`` is called from the same parent coroutine several times, then the
   order of ``then`` is the same if called in |JS|/|TS|.
   The callback is called on the coroutine when ``then`` called, and if
   ``Promise`` is passed from one coroutine to another and called ``then`` in
   both, then they are called in different coroutines (possibly concurrently).
   The developer must take care of possible data races.

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

.. _Unhandled Rejected Promises:

Unhandled Rejected Promises
===========================

In case of an unhandled rejection of ``Promise``, either the custom handler
provided for ``Promise`` rejection is called, or the default ``Promise``
rejection handler is called upon the entire program completion.

|

.. _Coroutines (Experimental):

*************************
Coroutines (Experimental)
*************************

.. meta:
    frontend_status: Partly

A function or lambda can be a *coroutine*. |LANG| supports *basic coroutines*,
*structured coroutines*.
Basic coroutines are used to create and launch a coroutine. The result is then
to be awaited. Details are provided in :ref:`Standard Library`.

.. index::
   structured coroutine
   basic coroutine
   coroutine

|
