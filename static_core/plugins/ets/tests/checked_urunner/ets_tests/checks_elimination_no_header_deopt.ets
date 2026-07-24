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

final class MyArray<T> extends Array<T> {
    constructor(arg: Array<T>) {
        super(arg.length, i => arg[i]);
    }
    update() {
        if (this.actualLength == 8) {
            this.actualLength = 0x7ffffffd
        } else {
            this.actualLength = 8
        }
    }
}

//! CHECKER         ChecksElimination does not create a deoptimization in header with SaveState from preheader
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options: "--compiler-regex='.*ETSGLOBAL::foo'"
//! METHOD          "checks_elimination_no_header_deopt.ETSGLOBAL::foo"
//! PASS_BEFORE_NTH "ChecksElimination", 2
//! IN_BLOCK        "prehead"
//! INST            "SaveStateDeoptimize"
//! IN_BLOCK        "head"
//! INST            /LoadObject.*std\.core\.Array\.actualLength/
//! PASS_AFTER_NEXT "ChecksElimination"
//! IN_BLOCK        "prehead"
//! INST            "SaveStateDeoptimize"
//! IN_BLOCK        "head"
//! INST_NOT        "DeoptimizeIf BOUNDS_CHECK"
//! RUN             entry: "checks_elimination_no_header_deopt.ETSGLOBAL::main"

function foo(arr: MyArray<int>) {
    let sum: int = 0
    for (let i: int = 0; i < arr.length.toInt(); i += 4) {
        sum += arr[i];
        arr[i + 3] = sum;
        arr.update()
    }
    return sum;
}

function main() {
    arktest.assertEQ(foo(new MyArray<int>([1, 2, 3, 4, 5, 6, 7, 8])), 6);
}