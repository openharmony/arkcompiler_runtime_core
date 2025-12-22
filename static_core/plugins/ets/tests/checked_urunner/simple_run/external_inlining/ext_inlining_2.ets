/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
  files: [ext_funcs.ets, ext_funcs_1.ets]
---*/

import { get_element } from "./ext_funcs_1.ets";

//! CHECKER    Check external inlining with depth 1 and two panda files
//! RUN_PAOC   options: "--panda-files=../../ext_inlining_2_ext_funcs.ets.abc:../../ext_inlining_2_ext_funcs_1.ets.abc --compiler-inline-external-methods-aot=true", entry: "ext_inlining_2.ETSGLOBAL::main"
//! EVENT      /Inline,ext_funcs_1.ETSGLOBAL::get_element,ext_funcs.ETSGLOBAL::add,.*,STATIC,SUCCESS/
//! EVENT_NEXT /Inline,ext_inlining_2.ETSGLOBAL::main,ext_funcs_1.ETSGLOBAL::get_element,.*,STATIC,SUCCESS/
//! RUN        options: "--panda-files=../../ext_inlining_2_ext_funcs.ets.abc:../../ext_inlining_2_ext_funcs_1.ets.abc", entry: "ext_inlining_2.ETSGLOBAL::main"

function main(): int {
    let arr: int[] = [1, 2, 3, 4];
    let val: int = get_element(arr, 2);
    return val === 6 ? 0 : 1;
}
