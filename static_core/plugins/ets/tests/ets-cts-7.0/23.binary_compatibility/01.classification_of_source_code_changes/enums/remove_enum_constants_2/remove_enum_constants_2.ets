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
desc: Removing enum constants may affect binary compatibility.
tags: [negative]
negative_steps: [runtime-with-new-libs.ark]
expected_error:
    runtime-with-new-libs.ark:
        - "E/verifier: Cannot link class: Cannot find field 'Red' in class 'remove_enum_constants_2.libr.Color'"
        - "E/verifier: Verifier error 55: Cache miss for field at offset"
        - "E/verifier: Verifier error 61: Cannot resolve field id"
        - "E/verifier: Method verification failed: void remove_enum_constants_2.ETSGLOBAL::main()"
        - "E/verifier: Class verification failed: 'remove_enum_constants_2.ETSGLOBAL' method: void remove_enum_constants_2.ETSGLOBAL::main()"
        - "E/runtime: Cannot initialize class 'remove_enum_constants_2.ETSGLOBAL'"
        - "Unhandled exception: std.core.LinkerVerificationError"
        - "LinkerVerificationError: Lremove_enum_constants_2/ETSGLOBAL;"
        - "at std.core.LinkerError.<ctor> (RuntimeLinkerErrors.ets:20:0)"
        - "at std.core.LinkerVerificationError.<ctor> (RuntimeLinkerErrors.ets:30:0)"
files: ['./remove_enum_constants_2.libr.ets']
---*/

import { Color } from './remove_enum_constants_2.libr.ets';

function main(): void {
    arktest.assertEQ(Color.Red, 0)
}
