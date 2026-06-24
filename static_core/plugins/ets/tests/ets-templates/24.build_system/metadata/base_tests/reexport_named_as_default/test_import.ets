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
    Consumer imports the proxy's default which is the origin's default
    passed through unchanged by `export { default } from '...'`.
files: [lib_origin.ets, lib_proxy.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "9"
---*/

import Calc from '@panda/proxy/lib_proxy'

console.log(new Calc().add(4, 5))
