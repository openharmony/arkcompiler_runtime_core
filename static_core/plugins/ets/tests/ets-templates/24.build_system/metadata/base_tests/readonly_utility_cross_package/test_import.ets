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
    Consumer imports Config and reads it through Readonly<Config>.
    The body intentionally only reads fields — verifying that
    AssignmentTarget allows a Config to flow into Readonly<Config>
    across packages.
files: [lib_config.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "localhost:80"
        - "example.com:443"
---*/

import { Config, makeConfig } from '@panda/lib/lib_config'

function inspect(r: Readonly<Config>): string {
    return r.host + ":" + r.port
}

console.log(inspect(new Config()))
console.log(inspect(makeConfig("example.com", 443)))
