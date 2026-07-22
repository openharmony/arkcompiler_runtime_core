/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
desc: >-
    Negative (metadata/GAP3): a proxy package re-exports a name that the source
    package did NOT export (Hidden). Must be a compile-time error (ESE0361).
files: [lib_origin.ets]
tags: [compile-only, negative]
name: test_reexport_n
arktsconfig: arktsconfig-proxy.json
---*/

export { Hidden as Leaked } from '@panda/origin/lib_origin'
