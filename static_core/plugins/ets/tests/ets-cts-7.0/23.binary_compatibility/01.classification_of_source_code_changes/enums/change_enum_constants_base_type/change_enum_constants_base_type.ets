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
desc: Changing enum base type may affect binary compatibility.
tags: [negative]
negative_steps: [runtime-with-new-libs.ark]
expected_error:
    runtime-with-new-libs.ark:
        - "There is no known error, test in KFL"
files: ['./change_enum_constants_base_type.libr.ets']
---*/

import { Color } from './change_enum_constants_base_type.libr.ets';

function main(): void {
    arktest.assertEQ(Color.Red, 2)
}
