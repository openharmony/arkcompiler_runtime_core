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


|

Most hardware in the modern world has multiple cores, and software must be
able to use more than one core in some scenarios (e.g., multimedia processing,
data analysis, simulation, modeling, databases etc.) to achieve maximum
performance. Besides, it is also crucial to provide support at different levels
for a number of various asynchronous APIs.

|

.. _Major Concurrency Features:

Major Concurrency Features
**************************

.. meta:
    frontend_status: Done

|LANG| has APIs for asynchronous programming that:

- Enables tasks to be suspended and resumed later, and
- Supports coroutines that run in parallel implicitly or explicitly.

Since the |LANG| coroutines share memory, a developer must be aware of possible
associated issues, and use appropriate functionality to guarantee thread safety.

|LANG| enables both asynchronous programming and parallel-run coroutines, and
provides machinery for trustworthy concurrent programs by providing the
following:

1. :ref:`Asynchronous features`: ``async`` / ``await`` / ``Promise``;
2. Coroutines (experimental) in :ref:`Standard Library`;
3. Structured concurrency in :ref:`Standard Library` (TaskPool API);
4. Synchronization primitives and 'thread'-safe containers in
   :ref:`Standard Library`.

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


