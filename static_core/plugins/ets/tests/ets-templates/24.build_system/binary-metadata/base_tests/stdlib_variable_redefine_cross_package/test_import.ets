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
    A consumer imports the redefined constant 'NaN' from the library. The
    imported (user) value 42 must be the one observed. EXPECTED once the
    feature works: output "42".
    CURRENT behaviour (bug, tracked by issue #25226): unlike the function
    case this compiles WITHOUT error, but the name silently resolves to the
    std/core 'NaN' rather than the imported constant, so it prints "NaN" at
    runtime — a wrong-value mismatch against the expected "42". The test is
    listed in ets-cts-ignored.txt under #25226 and will pass once the
    redefined entity is preferred on import.
files: [lib_const.ets]
name: test_import
arktsconfig: arktsconfig-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "42"
---*/

import { NaN } from '@panda/lib/lib_const'

console.log(NaN)
