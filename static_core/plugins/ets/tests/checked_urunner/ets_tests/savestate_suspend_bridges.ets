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

//! CHECKER       Test SaveStateSuspend bridge generation and saving via codeinfo AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-inlining-blacklist=savestate_suspend_bridges.ETSGLOBAL::bridgedInt," +
//!                                                      "savestate_suspend_bridges.ETSGLOBAL::bridgedDouble," +
//!                                                      "savestate_suspend_bridges.ETSGLOBAL::bridgedBool," +
//!                                                      "savestate_suspend_bridges.ETSGLOBAL::bridgedLong," +
//!                                                      "savestate_suspend_bridges.ETSGLOBAL::bridgedClass," +
//!                                                      "savestate_suspend_bridges.ETSGLOBAL::checkVals"
//! METHOD        "savestate_suspend_bridges.ETSGLOBAL::asyncFoo"
//! PASS_BEFORE   "FillSaveStateSuspendInputs"
//! INST_NOT      /SaveStateSuspend.*bridge.*bridge.*bridge.*bridge.*bridge/
//! PASS_AFTER    "FillSaveStateSuspendInputs"
//! INST          /SaveStateSuspend.*bridge.*bridge.*bridge.*bridge.*bridge/
//! RUN           entry: "savestate_suspend_bridges.ETSGLOBAL::main", options: "--coroutine-impl=stackless"

class Pair {
    ref: Object | undefined;
    primitive: long;
}

function bridgedInt() {
    return 1;
}

function bridgedDouble() {
    return 1.0;
}

function bridgedBool() {
    return false;
}

function bridgedLong(): long {
    return 10;
}

function bridgedClass(): Object {
    return "123"
}

let witness = new Object()

async function asyncFoo(p: Promise<Pair>): Promise<Pair> {
    const b1 = bridgedInt()
    const b2 = bridgedDouble()
    const b3 = bridgedBool()
    const b4 = bridgedLong()
    const b5 = bridgedClass() as string;
    witness = [
        (b1 + 1) as Object,
        (b2 + 1.0) as Object,
        (!b3) as Object,
        (b4 - 1) as Object,
        b5 as Object
    ] as FixedArray<Object>;
    let primitive = 42;
    let pair = await p;
    const ref = [
        bridgedClass() as string,  // bridge string class for CheckCast
        (b4 - 1) as Object,
        (b1 + 1) as Object,
        (b2 + 1.0) as Object,
        (!b3) as Object
    ] as FixedArray<Object>;
    checkVals(ref)
    pair.ref = ref
    pair.primitive = primitive;
    return pair;
}

function checkVals(arrObj: Object | undefined) {
    const arr = arrObj as FixedArray<Object>
    arktest.assertEQ(arr.length, 5)
    arktest.assertEQ(arr[0], "123")
    arktest.assertEQ(arr[1], 9 as long)
    arktest.assertEQ(arr[2], 2)
    arktest.assertEQ(arr[3], 2.0)
    arktest.assertEQ(arr[4], true)
}

function main() {
    let pair = new Pair();
    let p = Promise.resolve(pair);
    asyncFoo(p);
}