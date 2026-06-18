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
    arktsconfig 'extends' MULTI-STEP cycle crash repro. The consumer's config
    (-import.json) extends the library's config (-lib.json), which extends back
    to -import.json — a 2-hop A -> B -> A cycle (both configs are real test
    configs copied by the runner). ArkTsConfig::ParseExtends (util/arktsconfig.cpp:190)
    only rejects a DIRECT self-cycle (basePath != configPath, EAC0002); it keeps no
    visited-set / depth limit, so a multi-hop cycle recurses Parse() infinitely
    and es2panda SEGFAULTS (rc=139) — the library build hits it first. EXPECTED once
    fixed: a cyclic-import diagnostic (EAC0002) like the direct case, then output "1".
    Listed in ets-cts-ignored.txt (crash).
files: [lib_extends_cycle.ets]
name: test_extends_cycle
arktsconfig: arktsconfig-extends_cycle-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "1"
---*/
import { Cyc } from "@panda/lib/lib_extends_cycle"
console.log(new Cyc().v)
