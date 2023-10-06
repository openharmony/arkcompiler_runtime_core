..  Copyright (c) 2021-2023 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

How to Use the Cookbook
=======================

The main goal of this cookbook is to provide recipes for all partially
supported features and explicitly list all unsupported features.

The document is built on the feature-by-feature basis, and if you do not
find some feature, then you can safely consider it **supported**. Otherwise,
a recipe will give you a suggestion on how to rewrite your code and work
around an unsupported case.

|

Recipe Explained
----------------

The original |TS| code containing the keyword ``var``:

.. code-block:: typescript

    function addTen(x: number): number {
        var ten = 10
        return x + ten
    }

must be rewritten as follows:

.. code:: typescript

    // Important! This is still valid TypeScript.
    function addTen(x: number): number {
        let ten = 10
        return x + ten
    }

Status of Unsupported Features
------------------------------

Currently unsupported are mainly the features which:

- relate to dynamic typing that degrades runtime performance, or
- require extra support in the compiler, thus degrading project build time.

However, the |LANG| team reserves the right to reconsider the list and
**shrink** it in the future releases based on the feedback from the developers,
and on more real-world data experiments.

|

|


.. raw:: pdf

   PageBreak


