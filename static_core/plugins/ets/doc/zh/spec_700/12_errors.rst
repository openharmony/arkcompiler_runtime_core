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


.. _Error Handling:

错误处理
################################################################################

.. meta:
    frontend_status: Done

|LANG| 被设计为能够在程序中对不同的错误场景提供一等支持，以便进行响应与恢复。正常的程序执行可能会被以下两类情况中断：

- 运行时错误，例如空指针解引用、数组越界检查失败或除以零；
- 操作完成失败，例如从磁盘文件中读取并处理数据的任务，可能因为指定路径上文件不存在、没有读取权限等原因而失败。

本规范中的 *error* 一词表示所有类型的错误情形。

.. index::
   execution
   null pointer dereferencing
   runtime error
   array bounds checking
   completion
   normal execution
   normal completion
   completion failure
   path
   read permission
   error

|

.. _Errors:

错误
********************************************************************************

.. meta:
    frontend_status: Done

*Error* 是所有错误情形的基类。通常无需再定义新的错误类，因为:ref:`Standard Library` 中已经为各种常见情况定义了必要的错误类，例如 ``RangeError``。

不过，开发者仍然可以直接使用 ``Error`` 类本身，或使用 ``Error`` 的子类来处理新的错误情形。下面给出一个错误处理示例：

.. index::
   error
   base class
   class
   error handling
   derived class
   standard library


.. code-block:: typescript
   :linenos:

   class UnknownError extends Error { // user-defined error class 
      error: Error
      constructor (error: Error) {
         super()
         this.error = error
      }
    }

    function get_array_element<T>(array: T[], index: int): T|undefined {
        try {
          return array[index] // RangeError if index < 0 or index >= array.length
        }
        catch (error) {
          if (error instanceof RangeError) // invalid index detected
             return undefined
          throw new UnknownError (error) // unknown error occurred
        }
    }

    let arr = [1, 2, 3]
    let val = get_array_element(arr, -3) // RangeError: index -3 < 0

   console.log(val) // Output: undefined


在大多数情况下，错误由 |LANG| 运行时系统或 :ref:`Standard Library` 代码抛出。

新的错误情形也可以通过 ``throw`` 语句（见 :ref:`Throw Statements`）创建并抛出。错误可以通过 ``try`` 语句（见 :ref:`Try Statements`）处理。

.. note::
   某些错误无法恢复。

.. index::
   runtime system
   standard library
   generic class
   subclass
   error situation
   throw statement
   error
   try statement

.. code-block:: typescript
   :linenos:

    function handleAll(
      actions : () => void,
      handling_actions : () => void)
    {
      try {
        actions()
      }
      catch (x) { // Type of x is Error
          handling_actions()
      }
    }

.. raw:: pdf

   PageBreak
