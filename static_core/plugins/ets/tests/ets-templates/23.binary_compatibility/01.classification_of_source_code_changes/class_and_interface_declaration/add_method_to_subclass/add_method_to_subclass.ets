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
    Test for binary compatibility. Adding an overriding method to subclass
    doesn't break the binary compatibility. Added method is getting resolved
    by dynamic method resolution.
files: ['./add_method_to_subclass.libr.ets']
---*/

import { C1, C2 } from './add_method_to_subclass.libr.ets'

function main(): void {
    let c = ((): C1 => new C2())()
    arktest.assertEQ(c.m(23), c.version() == 1 ? "C1.m.23" : "C2.m.C1.m.23");
}
