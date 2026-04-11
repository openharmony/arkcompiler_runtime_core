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


|

|LANG| 提供了多种手段，用于编写能够通过并行与异步运行特定代码片段来高效利用硬件资源的程序。

本章涵盖：

- |LANG| 的执行模型；
- |LANG| 中 *异步*、*并行* 与 *并发* 代码执行的语义；
- 提供相应功能的语言特性与标准库 API。

|

.. _Execution model:

执行模型
********************************************************************************

.. meta:
    frontend_status: Done

|LANG| 程序定义一个或多个由运行时环境执行的 |C_JOBS|。|C_JOB| 是一段可以与其他 |C_JOBS| 并发执行的代码，并可通过语言提供的机制传递其返回值。

若目标平台支持并行代码执行，则 |C_WORKER| 是对平台所提供并行单元的抽象。通常它会与操作系统线程 1:1 对应。这意味着：

- 每个 |C_JOB| 都承载在某个 |C_WORKER| 上，并且任一时刻每个 |C_WORKER| 最多只执行一个 |C_JOB|；
- 如果两个或更多 |C_JOBS| 运行在不同的 |C_WORKERS| 上，那么它们的代码能够并行运行（这种执行模式称为 :term:`parallel execution`）；
- 如果多个 |C_JOBS| 共享同一个 |C_WORKER|，那么它们的代码永远不能并行运行（这种执行模式称为 :term:`asynchronous execution`）。

|C_JOB| 的主体有起点（对应代码片段的开始）和完成点（对应代码片段的结束）。|C_JOB| 的主体可以但不必与函数、方法或 lambda 等语言实体一一对应。

|C_JOB| 可以有零个或多个挂起点。|C_JOB| 的执行可以在挂起点暂停，并在稍后的某个时间恢复。挂起后，该 |C_JOB| 允许另一个 |C_JOB| 在同一 |C_WORKER| 上执行。

任意 |LANG| 程序都会隐式定义一个 **main** |C_JOB|，它对应
:ref:`Program Entry Point`。程序执行从它开始，而它的完成会触发程序终止序列。

用于定义 |C_JOBS| 及其相应挂起点的具体语言特性和标准库 API，将在后续各节描述。

程序内存由所有 |C_JOBS| 共享，这使得数据共享效率较高，但也意味着开发者应使用所提供的同步手段来避免数据竞争并保证线程安全。


.. _Overview of concurrency features:

并发特性概览
********************************************************************************

|LANG| 同时支持异步编程与并行编程，并提供以下内容：

- :ref:`Asynchronous execution` 原语：``async`` / ``await`` / ``Promise``；
- :ref:`Parallel Execution` API：``EAWorker API`` / ``Taskpool API``，包括结构化并发支持；
- :ref:`Synchronization` API：锁 API、原子操作 API 及其他同步手段。

:ref:`API details and restrictions` 一节给出了详细 API 说明以及使用限制。

.. index::
   concurrency
   asynchronous programming
   coroutine
   shared memory
   functionality
   parallel-run coroutine
   structured concurrency
   synchronization

|
