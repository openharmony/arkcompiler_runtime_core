..
    Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

Error Handling
##############

.. meta:
    frontend_status: Done

|LANG| is designed to provide first-class support in responding to, and
recovering from different error situations in a program. Normal program
execution can be interrupted by the occurrence of situations of two kinds:

-  Runtime errors (e.g., null pointer dereferencing, array bounds
   checking, or division by zero);

-  Operation completion failures (e.g., the task of reading
   and processing data from a file on disk can fail if the file does
   not exist on a specified path, read permissions are not available,
   or else).

The term *error* in this specification denotes all kinds of error situations.

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

|

.. _Errors:

Errors
******

.. meta:
    frontend_status: Done

*Error* is the base class of all error situations. Defining a new
error class is normally not required because essential error classes for
various cases (e.g., ``ArrayIndexOutOfBoundsError``) are defined in the
standard library (see :ref:`Standard Library`).

However, a developer can handle a new error situation by using ``Error``
class itself, or by a subclass of ``Error``. An example of error
handling is provided below:

.. index::
   error
   class
   error handling
   derived class

|

.. code-block:: typescript
   :linenos:

   class UnknownError extends Error { // User-defined error class 
      error: Error
      constructor (error: Error) {
         super()
         this.error = error
      }
    }

    function get_array_element<T>(array: T[], index: number): T|null {
        try {
          return array[index] // access array
        }
        catch (error) {
          if (error instanceof ArrayIndexOutOfBoundsError) // invalid index detected
             return null
          throw new UnknownError (error) // Unknown error occurred
        }
    }

In most cases, errors are raised by the |LANG| runtime system, or by the
standard library (see :ref:`Standard Library`) code.

New error situations can be created and raised by ``throw`` statements (see
:ref:`Throw Statements`) .

Errors are handled by using ``try`` statements (see :ref:`Try Statements`).

**Note**. Not every error can be recovered.

.. index::
   runtime system
   standard libraty
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
