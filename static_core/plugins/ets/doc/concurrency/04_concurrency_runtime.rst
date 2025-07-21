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

*********************************
Concurrency in the |LANG| runtime
*********************************

=============================
Worker threads and coroutines
=============================

For supporting `coroutines` in |LANG| we use pool of worker threads for running them. So we have M:N concurrency (M threads used for executing of N coroutines).

.. uml:: concurrency_worker_threads_and_coroutines_components.plantuml

There are some rules for different coroutines for migration, so some coroutines can have affinity to specific worker thread or to the parent coroutine.

=========
Scheduler
=========

The scheduler is responsible for fair scheduling of coroutines on the general Coroutine Worker threads. Also it guarantees that some coroutines which shouldn't
migrate will keep their domain and worker thread invariant.