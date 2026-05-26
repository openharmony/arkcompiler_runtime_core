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


.. _Runtime implementation details:

运行时实现细节
********************************************************************************

.. meta:
    frontend_status: Done

.. _Scheduling rules:

调度规则
================================================================================

.. meta:
    frontend_status: Done

运行时环境按照以下规则调度 |LANG| 程序中定义的 |C_JOBS|：

- 每个 |C_JOB| 都有一个只由其类型决定的优先级，按从高到低依次为：

  1. |C_COROS| 与 ``Promise`` 回调（``.then()`` 等）；
  2. 其他 |C_JOBS|。

- 在每个 |C_WORKER| 内，优先级更高的 |C_JOBS| 先于优先级更低的 |C_JOBS| 被调度；
- 所有优先级相同的 |C_JOBS| 都按 FIFO 顺序调度。

.. note::
   这些规则目前尚不完整，后续还会更新。

.. index::
   term1
   term 2

|
