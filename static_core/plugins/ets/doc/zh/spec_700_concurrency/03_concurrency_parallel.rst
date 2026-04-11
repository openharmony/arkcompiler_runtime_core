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


.. _Parallel Execution:

并行执行
********************************************************************************

.. meta:
    frontend_status: Done

:term:`parallel execution` 能力主要用于以下情形：开发者的代码需要执行可利用多 CPU 核心的密集型任务，或需要在独立操作系统线程中运行的长任务，以避免阻塞。

针对这些场景，|LANG| 提供了标准库级 API，用于以函数或方法粒度并行运行代码，也就是定义可以运行在不同 |C_WORKERS| 上的 |C_JOBS|，并支持指定 |C_JOBS| 之间的依赖关系，以及在可用 CPU 核心和/或操作系统线程之间平衡负载。

这种能力与 :ref:`异步执行 <Asynchronous execution>` 是正交的。也就是说，
异步函数同样可以并行运行，而且通常不会影响它们的挂起/恢复方式。
唯一的区别是：在某些条件下，|C_CORO| 在恢复时可能切换到另一个 |C_WORKER|。
具体规则见 :ref:`调度规则 <Scheduling rules>`。

.. _Parallel execution API:

并行执行 API
================================================================================

.. meta:
    frontend_status: Done

|LANG| 标准库为并行执行提供了以下几组 API：

- :ref:`EAWorker API`：创建由某个 |C_JOB| 及其子任务专用的 |C_WORKERS|；
- :ref:`Taskpool API`：提供结构化并发能力的框架，包括任务分组、依赖关系和取消机制。

|

.. _EAWorker API:

EAWorker API
================================================================================

EAWorker API 适用于开发者代码需要运行在专用 |C_WORKER| 上的场景，例如 UI 框架。

该 API 会为初始 |C_JOB| 的 *独占* 使用创建一个 |C_WORKER|。这意味着，该初始 |C_JOB| 及其派生出的所有 |C_JOBS| 都会停留在新创建的 |C_WORKER| 上，而其他任何 |C_JOB| 都不能被调度到该 |C_WORKER|。

更多信息请参见标准库文档。

.. index::
   EAWorker

|

.. _Taskpool API:

Taskpool API
================================================================================

``taskpool`` 是结构化并发框架。它允许创建新的 |C_JOBS|、指定它们之间的依赖关系、取消已派生的 |C_JOBS|、将其组合成组，并选择复杂的执行顺序。

.. note::
    由 taskpool API 创建的 |C_COROS| 在恢复时不能被重新调度到另一个 |C_WORKER|。

更多信息请参见标准库文档。

.. index::
   taskpool

|
