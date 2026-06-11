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
    Consumer imports Config from another package and applies Partial<T>
    locally. Three call sites cover: empty literal (all undefined),
    one-field literal (one defined / one undefined), full literal
    (all defined).
files: [lib_config.ets]
name: test_import
arktsconfig: arktsconfig-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "default:0:n"
        - "example.com:0:n"
        - "x:443:y"
---*/

import { Config } from '@panda/lib/lib_config'

function describe(c: Partial<Config>): string {
    let host: string = c.host !== undefined ? c.host! : "default"
    let port: int = c.port !== undefined ? c.port! : 0
    let verbose: string = c.verbose !== undefined ? (c.verbose! ? "y" : "n") : "n"
    return host + ":" + port + ":" + verbose
}

console.log(describe({}))
console.log(describe({host: "example.com"}))
console.log(describe({host: "x", port: 443, verbose: true}))
