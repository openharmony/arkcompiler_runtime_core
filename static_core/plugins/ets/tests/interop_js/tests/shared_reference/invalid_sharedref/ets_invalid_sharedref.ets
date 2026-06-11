/**
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

let module = ESValue.load(
    '../../plugins/ets/tests/interop_js/tests/shared_reference/invalid_sharedref/test_ts_invalid_sharedref');

// Test 1: verify SharedReference identity for JS arrays passed from TS
// The same array passed twice should be === equal (cached SharedReference)
let tmpArr: Array<number> = [];
function checkArrRefEquality(arr: Array<number>): boolean {
    let res: boolean = tmpArr === arr;
    tmpArr = arr;
    return res;
}

// Test 2: verify two different arrays are not === equal
function checkTwoNotEqual(a: Array<number>, b: Array<number>): boolean {
    let res: boolean = !(a === b);
    return res;
}

// Test 3: verify SharedReference identity for JS functions
let tmpFn: Function = () => {};
function checkFuncRef(fn: Function): boolean {
    let res: boolean = tmpFn === fn;
    tmpFn = fn;
    return res;
}

// Test 4: verify ESValue areEqual / areStrictlyEqual via module.getProperty
// ETS internally loads JS objects via module.getProperty and compares them
let jsArray1: ESValue;
let jsArray2: ESValue;
let o1: ESValue;
let o2: ESValue;
function checkAreEqual(): boolean {
    jsArray1 = module.getProperty('sharedObj');
    jsArray2 = module.getProperty('sharedObj');
    o1 = module.getProperty('o');
    o2 = module.getProperty('o');
    let res: boolean = ESValue.areEqual(jsArray1, jsArray2) && ESValue.areStrictlyEqual(jsArray1, jsArray2)
                       && ESValue.areEqual(o1, o2) && ESValue.areStrictlyEqual(o1, o2);
    return res;
}
