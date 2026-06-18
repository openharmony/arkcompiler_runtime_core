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
desc: consumer for the deepnest stress dimension (Array<…> nesting depth 1000, below the emission stack-overflow threshold); uses the companion Anchor class.
files: [lib_deepnest.ets]
name: test_deepnest
arktsconfig: arktsconfig-deepnest-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "1"
---*/
import { Anchor } from "@panda/lib/lib_deepnest"
import type { DeepNest } from "@panda/lib/lib_deepnest"
console.log(new Anchor().v)
