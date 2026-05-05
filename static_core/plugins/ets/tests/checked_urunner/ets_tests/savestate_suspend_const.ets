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

//! DEFINE_FRONTEND_OPTIONS DEFAULT ARGS
//! RUN_FRONTEND            options: "--stackless-coros=true"

//! CHECKER       Test SaveStateSuspend bridge generation and saving via codeinfo for constants AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-inlining-blacklist=savestate_suspend_const.ETSGLOBAL::checkVals"
//! METHOD        "savestate_suspend_const.ETSGLOBAL::asyncFoo"
//! PASS_BEFORE   "FillSaveStateSuspendInputs"
//! INST_NOT      /SaveStateSuspend.*bridge.*bridge/
//! PASS_AFTER    "FillSaveStateSuspendInputs"
//! # check that undefined is not passed as immediate
//! INST_NOT      /SaveStateSuspend.*0x0\(/
//! INST          /SaveStateSuspend.*bridge.*bridge/
//! RUN           entry: "savestate_suspend_const.ETSGLOBAL::main", options: "--coroutine-impl=stackless"

class Pair {
    ref: Any;
    primitive: long;
}

let witness: Any = null;

async function asyncFoo(p: Promise<Pair>): Promise<Pair> {
    witness = undefined;
    let primitive = 42;
    let pair = await p;
    pair.ref = witness === undefined ? pair : witness;
    pair.primitive = primitive;
    checkVals(pair, undefined)
    return pair;
}

function checkVals(p: Pair, und: Any) {
    arktest.assertEQ(und, undefined);
    if (p.ref !== p) {
        arktest.assertEQ(p.ref, undefined);
    }
    arktest.assertEQ(p.primitive, 42);
}

function main() {
    let pair = new Pair();
    let p = Promise.resolve(pair);
    asyncFoo(p);
}
