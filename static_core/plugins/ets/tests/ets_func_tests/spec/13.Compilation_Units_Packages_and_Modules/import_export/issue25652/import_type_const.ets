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
files:
- ./export_type_const.ets
---*/


import type { int_1,  int_2, int_3 } from "./export_type_const"
import { int_3 } from "./export_type_const"


import type { int_4 , int_5, int_6 } from "./export_type_const"
import { int_6 } from "./export_type_const"


function main(): void {
    let a1 = int_1;
    let a2 = int_2;
    let a3 = int_3;
    let a4 = int_4;
    let a5 = int_5;
    let a6 = int_6;
}
