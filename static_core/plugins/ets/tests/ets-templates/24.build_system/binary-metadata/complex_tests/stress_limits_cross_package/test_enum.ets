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
desc: consumer for the enum stress dimension (50000 members; an enum of >=65535 members overflows the 16-bit index region MAX_INDEX_16, so this stays well under).
files: [lib_enum.ets]
name: test_enum
arktsconfig: arktsconfig-enum-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "34999"
---*/
import { BigEnum } from "@panda/lib/lib_enum"
let e: BigEnum = BigEnum.E34999
console.log(e.valueOf())
