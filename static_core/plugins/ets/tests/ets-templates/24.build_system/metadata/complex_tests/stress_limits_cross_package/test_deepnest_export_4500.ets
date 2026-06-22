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
    Consumer of the deep-nest library. EXPECTED once the crash is fixed:
    output "1". CURRENT behaviour (at nesting depth 4500): the LIBRARY compilation segfaults
    (deep-nesting stack overflow in abc emission, no recursion guard outside
    the parser), so the library .abc is never produced and this test fails
    at the export step. Listed in ets-cts-ignored.txt.
files: [lib_deepnest_export_4500.ets]
name: test_deepnest_export_4500
arktsconfig: arktsconfig-deepnest_export_4500-import.json
expected_out:
    runtime:
        - "1"
---*/
import { Anchor } from "@panda/lib/lib_deepnest_export_4500"
let a = new Anchor()
console.log(a.v)
