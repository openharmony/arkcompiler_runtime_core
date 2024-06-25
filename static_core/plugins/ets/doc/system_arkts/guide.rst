..
    Copyright (c) 2024 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

How to Use System ArkTS
=======================

The main goal of System |LANG| is to help developers make their ArkTS code
more efficient.

System ArkTS provides short tips to boost performance - when you use a feature with bad
performance, System |LANG| warns you about it and suggests what else a developer
can use and how to rewrite the code.

All "System |LANG|" warnings are divided into the following categories:

#. **Subset warnings**: Part of |LANG| which is same as |TS|.
   The original code requires modifications that keep you within the Common Subset with |TS|.
#. **Non-subset warnings**: Part of |LANG| which is different with |TS|.
   The original code requires modifications which are not in Subset with |TS|.

List of possible options:

* **ets-subset-warnings**              : Enable all ETS-warnings that keep you in subset with |TS|.
* **ets-werror**                       : Treat all enabled ETS-warnings as errors.
* **ets-non-subset-warnings**          : Enable all ETS-warnings that are not in subset with |TS|.
* **ets-warnings-all**                 : Enable all ETS-warnings in System ArkTS.

* **ets-implicit-boxing-unboxing**     : Check if a program contains implicit boxing or unboxing. ETS Subset Warning.
* **ets-boost-equality-statement**     : Suggest boosting Equality Statements. ETS Subset Warning.
* **ets-phohibit-top-level-statements**: Prohibit Top-Level statements. ETS Subset Warning.
* **ets-remove-async**                 : Suggest replacing async functions with coroutines. ETS Non-subset Warning.
* **ets-suggest-final**                : Suggest final keyword warning. ETS Non-subset Warning.
* **ets-remove-lambda**                : Suggestions to replace lambda with regular functions. ETS Subset Warning.

|

To see System ArkTS warnings, just add System ArkTS options from the list above while compile.

Usage Example
----------------

#. Write some ArkTS code. For example, you have the following chunk of code:

.. code-block:: typescript

    class A {
        foo(): String {
            return "foo";
        };
    }

    class K extends A  {
        foo_to_suggest(): void {};
        override foo(): String {
            return "overridden_foo";
        }
    }

#. Provide an option to compiler. For example, add ``--ets-suggest-final`` or ``--ets-suggest-final=true``
#. Compile your file or project with added options.
#. Look through ETS-Warnings in output. For certain code System ArkTS will write this:

    * ``ETS Warning: Suggest 'final' modifier for class. [Test.ets:7:11]``

    * ``ETS Warning: Suggest 'final' modifier for method. [Test.ets:8:23]``

    * ``ETS Warning: Suggest 'final' modifier for method. [Test.ets:9:21]``

#. Rewrite your code as System ArkTS suggested. Rewritten |LANG| code: in certain case:

.. code:: typescript

    // Important! Shown feature is non-subset with TypeScript.
    class A {
        foo(): String {
            return "foo";
        };
    }

    final class K extends A {
        final foo_to_suggest(): void {};
        final override foo(): String {
            return "overridden_foo";
        }
    }

|

``ets-werror`` Usage Example
-------------------------------

In this part you can see how to treat System ArkTS warnings as errors.
Let us continue the previous example.

#. Write some ArkTS code. For example, you got this part:

.. code-block:: typescript

    class I { // No final - inheritance
    }

    class A extends I { // Suggest final
    }

#. Provide an option to compiler and enable ``ets-werror``. For example, add ``--ets-suggest-final --ets-werror``
#. Compile file or project with added options. A Compile Time Error happened
#. Look through ETS-Warnings in output. For certain code System ArkTS will write this:

    * ``System ArkTS. Warning treated as error: Suggest 'final' modifier for class [werror.ets:4:11]``

#. Rewrite your code as System ArkTS suggested. Rewritten |LANG| code: in certain case:

.. code:: typescript


    class I { // No final - inheritance
    }

    final class A extends I { // Suggest final
    }

|

Status of not implemented features
-----------------------------------

System ArkTS team is working on providng more and more performance-related
suggestions for ArkTS developers. In the near future we are going to
investigate possible performance leaks:

* Union usage
* Nullable types
* Rest parameters check VS using Array
* Non-throwing function

See status updates in next releases.

|
