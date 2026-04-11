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

API 细节与限制
********************************************************************************

.. meta:
    frontend_status: Done

本节描述前述并发 API 中值得注意的细节与限制。

.. _Using async API:

使用异步 API
================================================================================

在某些情况下，对 ``async`` 函数的调用需要等待其结果，但调用点位于非异步函数中。此时，调用方函数应转换为异步函数，并且有时这种转换链需要一直延续到 :ref:`Program Entry Point`。为此，|LANG| 支持 ``async`` 入口点函数。

.. note::
   也许这一节未来更适合放到 handbook 中。

|

.. _Promise API:

Promise 类 API
================================================================================

``Promise`` 类的正确使用受到一些重要限制。

``Promise`` 类实例可以在某个 |C_WORKER| 上的某个 |C_JOB| 内安全地被 ``await``，同时该实例也可以由同一 |C_WORKER| 或不同 |C_WORKER| 上的另一个 |C_JOB| 完成解析或拒绝。

``Promise`` 类允许注册回调，以便在 ``Promise`` 被解析和/或拒绝时调用。这通过调用 ``Promise`` 类的 ``.then()`` / ``.catch()`` / ``.finally()`` 方法实现。不过，这些方法具有如下使用限制：

- 已注册的回调会作为独立的 |C_JOB| 在注册它的同一 |C_WORKER| 上执行；
- 如果多个回调来自位于同一 |C_WORKER| 的 |C_JOBS|，则它们的执行顺序与注册顺序一致；
- 如果多个回调来自位于不同 |C_WORKERS| 的 |C_JOBS|，则只在各自 |C_WORKER| 内部定义执行顺序，不同 |C_WORKERS| 上的结果性 |C_JOBS| 之间不保证顺序。

关于 ``Promise`` 方法的更多细节，请参见标准库文档。

|

.. _Unhandled Rejected Promises:

未处理的被拒绝 Promise
================================================================================

.. meta:
    frontend_status: Done

.. note::
   未处理拒绝的语义会在稍后重新审视，待 |LANG| 并发子系统的设计完成后再定。

一个被拒绝的 ``Promise`` 若在某一时刻没有任何 ``await`` 等待它，也没有通过 ``.then()`` / ``.catch()`` 方法为其注册任何回调，则被认为是未处理的。

这个时刻在每个 |C_WORKER| 上分别定义，因此某个 ``Promise`` 实例只会在某个 |C_WORKER| 的上下文中被视为 *未处理拒绝*，而在其他 |C_WORKERS| 上仍可能是 *已处理* 的。

.. index::
   unhandled promise
   rejected promise
   unhandled rejection
   rejection handler
   call
   program completion

|

.. _Concurrency error handling:

错误处理策略
================================================================================

通常，|LANG| 程序中抛出的所有错误，要么应当允许开发者处理，要么就被视为未捕获错误，并启动程序终止序列。这一规则适用于任意 |C_WORKER| 上的任意 |C_JOB|。

|LANG| 程序中的 |C_JOB| 可以异常完成，即抛出错误。由于 |C_JOBS| 通过 ``Promise`` 类实例传递返回值，因此当 |C_JOB| 异常完成时，相应的 promise 会被拒绝，而原始错误不会立即被视为未捕获。

不过，也存在一些开发者无法处理这种拒绝的情况，例如：

- 抛错的 |C_JOB| 是由运行时环境创建的，且没有任何 promise 可以被 ``await`` 或通过 ``.then()`` / ``.catch()`` 回调处理；
- *main* |C_JOB| 抛出了错误。

在这些情况下，|C_JOB| 抛出的原始错误将被视为未捕获错误。

|
