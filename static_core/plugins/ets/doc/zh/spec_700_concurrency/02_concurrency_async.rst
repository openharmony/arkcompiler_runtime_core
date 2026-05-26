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


.. _Asynchronous execution:

异步执行
********************************************************************************

.. meta:
    frontend_status: Done

:term:`asynchronous execution` 能力针对这样一种情形：开发者代码经常需要等待外部事件
（例如网络、定时器或用户输入），或者内部事件（例如运行在另一 |C_WORKER| 上的
|C_JOB| 的状态更新）。对于这类场景，|LANG| 提供了一种机制，可挂起某个 |C_JOB|
的执行，将该 |C_JOB| 标记为阻塞并等待特定事件发生；当事件发生后，再恢复其执行。

|LANG| 中提供异步执行支持的特性包括：

- ``async`` 与 ``await`` 关键字，分别用于标记可挂起的（``asynchronous``）函数，以及这些函数内部的挂起点；
- 标准库中的 ``Promise`` 类，它抽象表示尚未完成的计算结果，并会在未来某个时间获得其值。

.. _Async Functions:

异步函数
================================================================================

.. meta:
    frontend_status: Done

*异步函数* 是一种可在其函数体内部定义挂起点的函数。非异步函数不能包含挂起点。

.. note::
   *挂起点* 是函数代码中的某个位置，在此处函数执行可以暂停（此时函数变为 *suspended*），并在未来某个时间 *恢复*。
   挂起意味着控制流会转移到别处，但所有局部函数状态（例如实参与局部变量的值）都会一直保留到恢复时为止。

异步函数应使用 ``async`` 修饰符标记，并返回泛型 :ref:`Concurrency Promise Class`
类的实例，该实例对返回值进行封装。``async`` 修饰符不是函数类型的一部分：从类型系统角度看，一个返回
``Promise`` 的 ``non-async`` 函数，与一个具有相同返回类型和参数的 ``async`` 函数没有区别。
作为 :ref:`Program Entry Point` 的函数也可以是异步的。

``async`` 函数的执行可以在由 :ref:`await Expression` 定义的挂起点暂停。如果发生挂起，
``async`` 函数会立即返回一个 *pending* 状态的 ``Promise`` 实例。若这是控制流遇到的第一个挂起点，
控制权会转移回异步函数的调用方，运行时环境会创建一个与该被挂起函数对应的新 |C_CORO|。随后，
根据 :ref:`Scheduling rules`，``async`` 函数会从暂停处恢复执行。若是第二个及后续挂起点，
挂起发生后，运行时环境会调度另一个 |C_JOB| 执行。被调度的 |C_JOB| 同样按照 :ref:`Scheduling rules`
选取。

异步函数的执行可通过正常返回完成，也可通过抛出错误完成。在这两种情况下，所得的值或错误都会被封装进
一个 ``Promise`` 类实例中，分别使其变为 resolved 或 rejected（详情见
:ref:`Concurrency Promise Class`）。

返回类型为 ``Promise<T>`` 的异步函数，可以显式返回一个 ``Promise<T>`` 实例
（此时返回值会“原样”返回），也可以返回一个类型为 ``T`` 的值，后者会自动装箱为
``Promise<T>`` 的实例。这两种写法都可作为 ``async`` 函数体内 ``return`` 语句的
``expression``（见 :ref:`return Statements` 和 :ref:`Return Type Inference`）。
这里的 ``T`` 是 :ref:`Type Any` 的子类型。如果 ``T`` 的类型为 ``void`` 或 ``undefined``
（见 :ref:`Type undefined or void`），那么与非异步函数一样，允许使用无参数的 ``return`` 语句。

.. note::
   总结一下：带有一个或多个挂起点的异步函数，会在 |LANG| 程序中定义一个新的 |C_CORO|，
   它从第一次挂起开始，并在异步函数完成时结束。没有挂起点的异步函数不会定义任何额外的 |C_JOBS|。

若出现以下情形，则会发生 :index:`compile-time error`：

- 在静态初始化中调用 ``async`` 函数；
- ``async`` 函数带有 ``abstract`` 或 ``native`` 修饰符；
- ``async`` 函数的返回类型不是 ``Promise``；
- ``non-async`` 函数定义了任意挂起点。

.. index::
   async function
   coroutine
   return type
   static initializer
   abstract function
   native function
   function body
   backward compatibility
   annotation
   no-argument return statement
   async function
   return statement
   compatibility

|

.. _Async Lambdas:

异步 lambda
================================================================================

.. meta:
    frontend_status: Done

lambda 可以带有 ``async`` 修饰符（见 :ref:`Lambda Expressions` 和
:ref:`Trailing Lambdas`）。

在并发语义方面，``async`` lambda 与 :ref:`Async Functions` 具有相同语义，并遵循相同规则。

.. index::
   async lambda
   async modifier
   lambda expression
   coroutine

|

.. _Concurrency Async Methods:

异步方法
================================================================================

.. meta:
    frontend_status: Done

类的静态方法或实例方法可以带有 ``async`` 修饰符
（见 :ref:`Method Declarations`）。

在并发语义方面，``async`` 方法与 :ref:`Async Functions` 具有相同语义，并遵循相同规则。

.. index::
   async method
   class method
   async modifier
   method declaration
   coroutine

|

.. _await Expression:

``await`` 表达式
================================================================================

.. meta:
    frontend_status: Done

*await 表达式* 在异步函数体内部定义一个挂起点。其语法如下：

.. code-block:: abnf

    awaitExpression:
        'await' expression
        ;

这里的 ``expression`` 实参可以是任意类型（:ref:`Type Any`）。*await 表达式* 的类型是
``Awaited<type(expression)>``（见 :ref:`Awaited Utility Type`），但其值与语义取决于其
``expression`` 实参的类型。

如果 ``expression`` 的类型是 :ref:`Concurrency Promise Class` 的子类型，则：

- 包含它的异步函数执行会暂停，直到被等待的 ``Promise`` 实例变为 *resolved* 或 *rejected*；
- 如果被等待的 ``Promise`` 实例为 *resolved*，则用于解析该 ``Promise`` 的值成为 *await 表达式* 的值；
- 如果被等待的 ``Promise`` 实例为 *rejected*，则 *await 表达式* 会抛出用以拒绝该 ``Promise`` 实例的错误；

如果 ``expression`` 的类型不是 :ref:`Concurrency Promise Class` 的子类型，则 *await 表达式*
返回 ``expression`` 实参的值，且其所在异步函数不会被挂起：

.. code-block:: typescript
   :linenos:

   class SomeClass {
     method(): SomeClass | undefined { /* body */ }
     async asyncMethod(): Promise<string> { /* body */ }
   }

   async function g(): Promise<Object> { /* returns Promise */ }

   async function f() { // await is allowed in async context only
     // ...

     // v1 is Awaited<Promise<Object>>, which is effectively Object
     // g returns Promise, hence f can be suspended potentially
     let v1 = await g()

     // v2 is Awaited<Int>, which is effectively Int
     // await argument is an Int, hence no suspension occurs
     let v2 = await new Int(5)

     // v3 is Awaited<Promise<string> | undefined>, which is (string | undefined)
     // - if method() returned an object: suspension can occur, v3 is the await result
     // - if method() returned undefined: no suspension, v3 is undefined
     let v3 = await (new SomeClass).method()?.asyncMethod()

     // ...
   }

在某些条件下，在 ``await`` 上被挂起的 |C_CORO| 在恢复时可以移动到另一个 |C_WORKER| 上，
即发生重新调度（见 :ref:`Scheduling rules`）。

如果在异步函数、方法或 lambda 体之外使用 ``await``，则会发生 :index:`compile-time error`。

.. index::
   syntax
   await expression
   subtype
   expression
   resolution
   async function

|

.. _Concurrency Promise Class:

``Promise`` 类
================================================================================

.. meta:
    frontend_status: Done

``Promise`` 类表示某个将在未来某个时间确定的值，因此允许引用尚未完成的计算结果或未完成任务的结果。
|LANG| 中各种 |C_JOBS| 都使用 promise 将其结果传递给客户端代码。

``Promise`` 实例可以具有以下状态：

- *pending*：表示结果值尚未确定；
- *resolved*：表示该 ``Promise`` 已被 *fulfilled*，其值已经确定；
- *rejected*：表示关联计算异常完成，因此该 ``Promise`` 实例包含描述异常完成原因的错误实例；

获取用于解析或拒绝 ``Promise`` 的值的唯一方式，是对该 ``Promise`` 实例应用 :ref:`await Expression`。

.. note::
   如果 ``Promise`` 是由 **main** |C_WORKER| 上的异步函数返回，或在 **main** |C_WORKER| 上手动创建，
   那么其语义与 |JS|/|TS| 中 ``Promise`` 的语义类似。
   是否应将此类说明纳入 |LANG| 规范，仍有待确定。

.. index::
   promise object
   asynchronous API
   asynchronous operation
   API
   semantics
   proxy
   coroutine
   context
   async function
   qualification
   root coroutine

通常，``Promise`` 实例可以被并发安全地访问。此规则的例外情况以及详细 API 见
:ref:`API details and restrictions` 一节。

|
