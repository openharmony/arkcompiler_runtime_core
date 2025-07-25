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
'./export_default_anonymous/export_newclass.ets',
'./export_default_anonymous/export_multi_expressions.ets',
'./export_default_anonymous/export_lambda.ets',
'./export_default_anonymous/export_array.ets',
'./export_default_anonymous/export_add_expressions.ets']
---*/

import { default as export_string } from "./export_default_anonymous/export_string.ets";
import { default as export_number } from "./export_default_anonymous/export_number.ets";
import { default as export_newclass } from "./export_default_anonymous/export_newclass.ets";
import { default as export_multi_expressions } from "./export_default_anonymous/export_multi_expressions.ets";
import { default as export_lambda } from "./export_default_anonymous/export_lambda.ets";
import { default as export_array } from "./export_default_anonymous/export_array.ets";
import { default as export_add_expressions } from "./export_default_anonymous/export_add_expressions.ets";

function main() {
    assertEQ(export_string, "export_string")

    assertEQ(export_number, 123456)

    assertEQ(export_newclass.foo(), "export_newclass_A_foo is invoked")
    assertEQ(export_newclass.field, "export_newclass_A_field")

    assertEQ(export_multi_expressions, 110)

    assertEQ(export_lambda(), "export_lambda is invoked")

    assertEQ(export_array[0], 1)

    assertEQ(export_add_expressions, 21)
}
