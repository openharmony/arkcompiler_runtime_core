/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc:  The directive in the example below exports constant variable of some type anonymously.
files: ['./export_default_anonymous/export_string.ets', 
'./export_default_anonymous/export_number.ets',
'./export_default_anonymous/export_newclass.ets']
---*/

import { default as export_string, a as a1, b as b1 } from "./export_default_anonymous/export_string.ets";
import { default as export_number, a as a2, b as b2 } from "./export_default_anonymous/export_number.ets";
import { default as export_newclass, a as a3, b as b3 } from "./export_default_anonymous/export_newclass.ets";

function main() {
    assertEQ(export_string, "export_string")
    assertEQ(a1,100)
    assertEQ(b1,"export_string")

    assertEQ(export_number, 123456)
    assertEQ(a2,100)
    assertEQ(b2,"export_string")

    assertEQ(export_newclass.foo(), "export_newclass_A_foo is invoked")
    assertEQ(export_newclass.field, "export_newclass_A_field")
    assertEQ(a2,100)
    assertEQ(b2,"export_string")
}
