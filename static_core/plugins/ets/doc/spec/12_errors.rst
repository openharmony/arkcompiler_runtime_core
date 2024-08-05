..
    Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
recovering from different erroneous conditions in a program.

Two kinds of situations can occur and interrupt normal program
execution:

-  Runtime errors (e.g., null pointer dereferencing, array bounds
   checking, or division by zero);

-  Operation completion failures (e.g., the task of reading
   and processing data from a file on disk can fail if the file does
   not exist on a specified path, read permissions are not available,
   or else).


.. index::
   execution
   null pointer dereferencing
   runtime error
   array bounds checking
   completion
   normal execution
   normal completion
   completion failure

This specification uses the terms as follows:

-  *Error* to denote runtime errors, and

-  *Exception* to denote failures.


The difference between these two terms is that *exceptions* are the
*normal* and expected way for an operation to complete. A program
is expected to resolve some exceptions, and inform the user if it
cannot.

On the contrary, *errors* indicate that there is a failure of the
program logic, or even of the hardware. The program can recover in
some but not all cases.

As a result, exceptions can be handled in a much more effective
manner than errors.

.. index::
   error
   exception
   runtime

Some modern programming languages support only exceptions; others
support only errors. |LANG| is based on the presumption that both
*exceptions* and *errors* must be supported. ``Exception`` and
``Error`` as predefined types are discussed below.

Exceptions are described in the chapter Experimental Features (see
:ref:`Exceptions`) of this specification.

.. index::
   exception
   error
   predefined type

|

.. _Errors:

Errors
******

.. meta:
    frontend_status: Done

``Error`` is the base class of all errors. Defining a new error class is
normally not required because essential error classes for various cases (e.g.,
``ArrayIndexOutOfBoundsError``) are defined in the standard library (see
:ref:`Standard Library`).

However, a developer can define a new error by using ``Error``, or any
derived class as the base of the new class. An example of error handling is
provided below:

.. index::
   error
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
          throw new UnknownError (error as Error) // Unknown error occurred
        }
    }


A :index:`compile-time error` occurs if a generic class is directly or
indirectly a subclass of ``Error``.

In most cases, errors are raised by the |LANG| runtime system, or by the
standard library (see :ref:`Standard Library`) code.

The ``throw`` statements (see :ref:`Throw Statements`) allow throwing both
exceptions and errors. Throwing exceptions provide a structured way to
handle a range of unexpected situations in the application code. Throwing
errors in such a context is not recommended.

The ``try`` statements (see :ref:`Try Statements`) are used to handle
errors in a manner similar to the handling of exceptions.

**Note**: Not every error can be recovered.

.. index::
   compile-time error
   generic class
   subclass
   error
   throw statement
   exception
   error
   try statement

.. code-block:: typescript
   :linenos:

    class Exception extends Error {}

    function handleAll(
      actions : () => void,
      error_handling_actions : () => void,
      exception_handling_actions : () => void)
    {
      try {
        actions()
      }
      catch (x) {
        if (x instanceof Exception)
          exception_handling_actions()
        else if (x instanceof Error)
          error_handling_actions()
      }
    }


.. raw:: pdf

   PageBreak


