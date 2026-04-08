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
    If the Effective Type of the one Method or Function declaration, field
    or variable is not affected, the change does not affect binary compatibility.
files: ['./effective_type_interface_method.libr.ets']
---*/

import { I } from './effective_type_interface_method.libr.ets';

class A implements I {
    a(p: "abc"): "abc" { return p }
}

function main(): void {
    arktest.assertEQ(new A().a("abc"), "abc")
}
