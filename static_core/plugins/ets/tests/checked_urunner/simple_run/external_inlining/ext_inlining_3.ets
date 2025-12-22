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
  files: [ext_funcs_2.ets]
---*/

import { add } from "./ext_funcs_2.ets";
import { add_with_log } from "./ext_funcs_2.ets";
import { rand_plus } from "./ext_funcs_2.ets";
import { get_arr_element } from "./ext_funcs_2.ets";
import { make_array } from "./ext_funcs_2.ets";
import { stringify } from "./ext_funcs_2.ets";
import { safe_div } from "./ext_funcs_2.ets";

//! CHECKER    Check external inlining without side effects
//! RUN_PAOC   options: "--panda-files=../../ext_inlining_3_ext_funcs_2.ets.abc --compiler-inline-external-methods-aot=true", entry: "ext_inlining_3.ETSGLOBAL::main_1"
//! EVENT      /Inline,ext_inlining_3.ETSGLOBAL::main_1,ext_funcs_2.ETSGLOBAL::add,.*,STATIC,SUCCESS/
//! RUN        options: "--panda-files=../../ext_inlining_3_ext_funcs_2.ets.abc", entry: "ext_inlining_3.ETSGLOBAL::main_1"

function main_1(): int {
    let result = 0;

    for (let i = 0; i < 5; i++) {
        result += add(i, i);  // 0+0 + 1+1 + 2+2 + 3+3 + 4+4 = 20
    }

    if (result === 20) {
        return 0;
    } else {
        return 1;
    }
}

//! CHECKER    Check external inlining function with runtime call from std core
//! RUN_PAOC   options: "--panda-files=../../ext_inlining_3_ext_funcs_2.ets.abc --compiler-inline-external-methods-aot=true", entry: "ext_inlining_3.ETSGLOBAL::main_2"
//! EVENT      /Inline,ext_inlining_3.ETSGLOBAL::main_2,ext_funcs_2.ETSGLOBAL::add_with_log,.*,STATIC,SUCCESS/
//! RUN        options: "--panda-files=../../ext_inlining_3_ext_funcs_2.ets.abc", entry: "ext_inlining_3.ETSGLOBAL::main_2"

function main_2(): int {
    let result = 0;

    for (let i = 0; i < 5; i++) {
        result += add_with_log(i, i);  // 0+0 + 1+1 + 2+2 + 3+3 + 4+4 = 20
    }

    if (result === 20) {
        return 0;
    } else {
        return 1;
    }
}

//! CHECKER    Check inlining of external function with runtime call of managed func
//! RUN_PAOC   options: "--panda-files=../../ext_inlining_3_ext_funcs_2.ets.abc --compiler-inline-external-methods-aot=true", entry: "ext_inlining_3.ETSGLOBAL::main_3"
//! EVENT      /Inline,ext_inlining_3.ETSGLOBAL::main_3,ext_funcs_2.ETSGLOBAL::rand_plus,.*,STATIC,SUCCESS/
//! RUN        options: "--panda-files=../../ext_inlining_3_ext_funcs_2.ets.abc", entry: "ext_inlining_3.ETSGLOBAL::main_3"

function main_3(): int {
    let ok = true;
    for (let i = 0; i < 5; i++) {
        let result = rand_plus(5);
        if (result < 5 || result > 14) {
            ok = false;
        }
    }

    return ok ? 0 : 1;
}

//! CHECKER    Check external inlining with possible ArrayIndexOutOfBounds
//! RUN_PAOC   options: "--panda-files=../../ext_inlining_3_ext_funcs_2.ets.abc --compiler-inline-external-methods-aot=true", entry: "ext_inlining_3.ETSGLOBAL::main_4"
//! EVENT      /Inline,ext_inlining_3.ETSGLOBAL::main_4,ext_funcs_2.ETSGLOBAL::get_arr_element,.*,STATIC,SUCCESS/
//! RUN        options: "--panda-files=../../ext_inlining_3_ext_funcs_2.ets.abc", entry: "ext_inlining_3.ETSGLOBAL::main_4"

function main_4(): int {
    let arr: int[] = [1, 2, 3, 4];
    let val: int = get_arr_element(arr, 3);
    return val === 4 ? 0 : 1;
}

//! CHECKER    Check external inlining with heap allocation
//! RUN_PAOC   options: "--panda-files=../../ext_inlining_3_ext_funcs_2.ets.abc --compiler-inline-external-methods-aot=true", entry: "ext_inlining_3.ETSGLOBAL::main_5"
//! EVENT      /Inline,ext_inlining_3.ETSGLOBAL::main_5,ext_funcs_2.ETSGLOBAL::make_array,.*,STATIC,SUCCESS/
//! RUN        options: "--panda-files=../../ext_inlining_3_ext_funcs_2.ets.abc", entry: "ext_inlining_3.ETSGLOBAL::main_5"

function main_5(): int {
    let arr: int[] = make_array(3);
    return arr[2] === 0 ? 0 : 1;
}

//! CHECKER    Check external inlining with virtual call
//! RUN_PAOC   options: "--panda-files=../../ext_inlining_3_ext_funcs_2.ets.abc --compiler-inline-external-methods-aot=true", entry: "ext_inlining_3.ETSGLOBAL::main_6"
//! EVENT      /Inline,ext_inlining_3.ETSGLOBAL::main_6,ext_funcs_2.ETSGLOBAL::stringify,.*,STATIC,SUCCESS/
//! RUN        options: "--panda-files=../../ext_inlining_3_ext_funcs_2.ets.abc", entry: "ext_inlining_3.ETSGLOBAL::main_6"

function main_6(): int {
    let num: int = 42;
    let s = stringify(num);
    if (s === "42") {
        return 0;
    } else {
        return 1;
    }
}

//! CHECKER    Check external inlining with possible ZeroDivision
//! RUN_PAOC   options: "--panda-files=../../ext_inlining_3_ext_funcs_2.ets.abc --compiler-inline-external-methods-aot=true", entry: "ext_inlining_3.ETSGLOBAL::main_7"
//! EVENT      /Inline,ext_inlining_3.ETSGLOBAL::main_7,ext_funcs_2.ETSGLOBAL::safe_div,.*,STATIC,SUCCESS/
//! RUN        options: "--panda-files=../../ext_inlining_3_ext_funcs_2.ets.abc", entry: "ext_inlining_3.ETSGLOBAL::main_7"

function main_7(): int {
    let a = 10;
    let b = 5;

    let result = safe_div(a, b);
    if (result === 2) {
        return 0;
    } else {
        return 1;
    }
}
