..
    Copyright (c) 2021-2023 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

.. _Implementation Details:

Implementation Details
######################

.. meta:
    frontend_status: None

Some implementation details are temporarily placed in this section.

.. _Import Path Lookup:

Import Path Lookup
******************

If an import path ``<some path>/name`` is resolved to a path in a folder "name",
then the following lookup sequence is executed by the compiler:

-   If the folder contains ``index.ets`` file, then this file is imported
    as a separate module written in |LANG|;

-   If the folder contains ``index.ts`` file, then this file is imported
    as a separated module written in |TS|;

-   Otherwise, the compiler imports a package constituted by files
    ``name/*.ets``.





