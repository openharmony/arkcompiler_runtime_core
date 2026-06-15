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
    Test for binary compatibility. The case when adding a final modifier to exported class
    doesn't break the binary compatibility.
files: ['./add_final_modifier.libr.ets']
---*/

import { C1 } from './add_final_modifier.libr.ets'

function main(): void {
    arktest.assertEQ(new C1().m(42), "C1.m.42");
}
