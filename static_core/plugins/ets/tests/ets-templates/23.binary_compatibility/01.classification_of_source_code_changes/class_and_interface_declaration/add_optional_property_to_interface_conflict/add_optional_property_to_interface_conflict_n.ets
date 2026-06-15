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
    Test for binary compatibility. Adding an optional property to interface
    may break the binary compatibility.
files: ['./add_optional_property_to_interface_conflict_n.libr.ets']
tags: [negative]
negative_steps: [verify-new-libs, runtime-with-new-libs.ark]
expected_error:
    verify-new-libs: |-
        TODO: Expected error message that added optional property breaks the binary compatibility
    runtime-with-new-libs.ark: |-
        TODO: Expected error message that added optional property breaks the binary compatibility
---*/

import { I1 } from './add_optional_property_to_interface_conflict_n.libr.ets'

class C2 implements I1 {
    prop: number = 27.0
}

let v = new C2().prop
