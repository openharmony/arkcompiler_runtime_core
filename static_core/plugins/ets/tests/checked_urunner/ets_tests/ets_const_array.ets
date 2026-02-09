/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

//! CHECKER         Test ConstArrayResolver for array of std.core.String
//! RUN_BCO         method: "ets_const_array.ETSGLOBAL.constArrayStr"
//! METHOD          "constArrayStr"
//! PASS_AFTER      "IrBuilder"
//! INST            "NewArray"
//! INST_COUNT      "LoadString", 3
//! INST_COUNT      "StoreArray", 3
//! PASS_AFTER      "ConstArrayResolver"
//! PASS_AFTER_NEXT "Cleanup"
//! INST            "LoadConstArray"
//! INST_NOT        "NewArray"
//! INST_NOT        "LoadString"
//! INST_NOT        "StoreArray"
//! BC_METHOD       "ets_const_array.ETSGLOBAL.constArrayStr"
//! INST            "lda.const"
//! INST_NOT        "newarr"
//! RUN             entry: "ets_const_array.ETSGLOBAL::testConstArrayStr"

function constArrayStr(idx: int): String {
    let a: FixedArray<String> = ['ab', 'cd', 'ef'];
    return a[idx];
}

function testConstArrayStr(): int {
    if (constArrayStr(1) != 'cd') {
        return 1;
    }
    try {
        constArrayStr(3);
    } catch(e) {
        return 0;
    }
    return 2;
}

//! CHECKER         Test ConstArrayResolver for ArkTS int array
//! RUN_BCO         method: "ets_const_array.ETSGLOBAL.constArrayInt"
//! METHOD          "constArrayInt"
//! PASS_AFTER      "IrBuilder"
//! INST            "NewArray"
//! INST_COUNT      "StoreArray", 3
//! PASS_AFTER      "ConstArrayResolver"
//! INST            "LoadConstArray"
//! INST_NOT        "NewArray"
//! INST_NOT        "StoreArray"
//! BC_METHOD       "ets_const_array.ETSGLOBAL.constArrayInt"
//! INST            "lda.const"
//! INST_NOT        "newarr"
//! RUN             entry: "ets_const_array.ETSGLOBAL::testConstArrayInt"

function constArrayInt(idx: int): int {
    let a: FixedArray<int> = [5, 8, 13];
    return a[idx];
}

function testConstArrayInt(): int {
    if (constArrayInt(1) != 8) {
        return 1;
    }
    try {
        constArrayInt(3);
    } catch(e) {
        return 0;
    }
    return 2;
}