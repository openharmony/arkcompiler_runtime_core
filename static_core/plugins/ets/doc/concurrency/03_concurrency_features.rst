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

###################################
Language level concurrency features
###################################

***************
async functions
***************

.. _async functions:

.. note::
  more verbose description you can find in the |LANG| language specification

The function *async* is implicitly a coroutine that can be called as a regular function.

By default *async* function, is :ref:`AJCoroutine<AJcoroutine>`

The return type of an *async* function must be *Promise<T>* (see
:ref:`Promise<Concurrency Promise Class>`).
Returning values of types *Promise<T>* and *T* from the function *async*
is allowed.

Using return statement without expression is allowed if the return type
is *Promise<void>*.
*No-argument* return statement can be implicitly added as the last statement
of the function body if there is no explicit return statement in a function
with the return type *Promise<void>*.

**Note**: Using this annotation is not recommended because this type of
functions is only supported for the sake of backward TypeScript compatibility.

.. index::
   function async
   coroutine
   return type
   function body
   backward compatibility
   annotation

*****
await
*****

.. _await:

``await`` is used for waiting :ref:`Promise<Concurrency Promise Class>`

If the Promise not resolved current coroutine will be suspended till it be resolved.

If the :ref:'Promise<Concurrency Promise Class>' is rejected, then the reason of rejection will be thrown. 

``await`` is forbidden to use outside of :ref:`async functions<async functions>`
