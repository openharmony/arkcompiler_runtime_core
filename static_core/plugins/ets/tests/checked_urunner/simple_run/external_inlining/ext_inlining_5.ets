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
  files: [ext_funcs_4.ets, ext_funcs_4_1.ets]
---*/

import { middle } from "./ext_funcs_4_1.ets";

//! CHECKER    Check external inlining with deoptimization
//! RUN_PAOC   options: "--panda-files=../../ext_inlining_5_ext_funcs_4.ets.abc:../../ext_inlining_5_ext_funcs_4_1.ets.abc --compiler-inline-external-methods-aot=true", entry: "ext_inlining_5.ETSGLOBAL::main_1"
//! EVENT      /Inline,ext_funcs_4_1.ETSGLOBAL::middle,ext_funcs_4.ETSGLOBAL::risky_array,.*,STATIC,SUCCESS/
//! EVENT      /Inline,ext_inlining_5.ETSGLOBAL::main_1,ext_funcs_4_1.ETSGLOBAL::middle,.*,STATIC,SUCCESS/
//! RUN        options: "--panda-files=../../ext_inlining_5_ext_funcs_4.ets.abc:../../ext_inlining_5_ext_funcs_4_1.ets.abc", entry: "ext_inlining_5.ETSGLOBAL::main_1"
//! EVENT      /DeoptimizationReason,ext_funcs_4.ETSGLOBAL::risky_array,BOUNDS_CHECK_WITH_DEOPT/
//! EVENT      /Deoptimization,ext_inlining_5.ETSGLOBAL::main_1,.*,IFRAME/

function main_1(): int {
    let arr: int[] = [1, 2, 3];
    let total = 0;

    try {
        total = middle(arr); // inlining: main → middle → risky_array
    } catch (e) {
        total = -1;
    }

    if (total == -1) {
        return 0;
    } else {
        return 1;
    }
}
