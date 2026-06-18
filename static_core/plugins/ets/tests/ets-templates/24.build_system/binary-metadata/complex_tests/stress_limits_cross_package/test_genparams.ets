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
desc: consumer for the genparams stress dimension (value kept under 1m20s compile).
files: [lib_genparams.ets]
name: test_genparams
arktsconfig: arktsconfig-genparams-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "7"
---*/
import { ManyGenerics, manyParams } from "@panda/lib/lib_genparams"
let g = new ManyGenerics()
console.log(g.v)
