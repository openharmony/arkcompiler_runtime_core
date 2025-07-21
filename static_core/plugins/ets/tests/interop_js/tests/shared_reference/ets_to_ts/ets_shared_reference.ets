/**
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

let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/shared_reference/ets_to_ts/test_ts_shared_reference');

let tmpFunc1: Function =  () => {};
function areFuncsEqual1(func: Function){
    let res: boolean = tmpFunc1 === func;
    tmpFunc1 = func;
    return res;
}

let tmpFunc2: ()=>void =  () => {};
function areFuncsEqual2(func: ()=>void){
    let res: boolean = tmpFunc2 === func;
    tmpFunc2 = func;
    return res;
}

let tmpArray: Array<number> = [1];
function areArraysEqual1(arr: Array<number>){
    let res: boolean = tmpArray === arr;
    tmpArray = arr;
    return res;
}

function areArraysEqual2(arr1: Array<number>, arr2: Array<number>){
    let res: boolean = arr1 == arr2;
    return res;
}

let jsArray1: ESValue;
let jsArray2: ESValue;
function areArraysEqual3(arr1: Array<number>){
    jsArray1 = module.getProperty('jsArray');
    jsArray2 = module.getProperty('jsArray');
    let o1 =  module.getProperty('o');
    let o2 =  module.getProperty('o');
    let res: boolean = ESValue.areEqual(jsArray1, jsArray2) && ESValue.areStrictlyEqual(jsArray1, jsArray2) 
                       && ESValue.areEqual(o1, o2) && ESValue.areStrictlyEqual(o1, o2)
                       && ESValue.areEqual(jsArray1, new ESValue(arr1)) && !(jsArray1 === new ESValue(arr1));
    return res;
}

const strTime = '2024-03-01T01:02:03.000Z';
let tmpData: Date = new Date(strTime);
function areDatasEqual(data: Date) {
    let res: boolean = tmpData === data;
    tmpData = data;
    return res;
}

let tmpMap: Map<string, number> = new Map<string, number>();
function areMapsEqual(map: Map<string, number>) {
    let res: boolean = tmpMap === map;
    tmpMap = map;
    return res;
}

let tmpSet: Set<number> = new Set<number>();
function areSetsEqual(set: Set<number>) {
    let res: boolean = tmpSet === set;
    tmpSet = set;
    return res;
}

let tmpRangeError: RangeError = new RangeError('tmpRange error example');
function areRangeErrorEqual(error: RangeError) {
    let res: boolean = tmpRangeError === error;
    tmpRangeError = error;
    return res;
}

let tmpSyntaxError: SyntaxError = new SyntaxError('tmpSyntax error example');
function areSyntaxErrorEqual(error: SyntaxError) {
    let res: boolean = tmpSyntaxError === error;
    tmpSyntaxError = error;
    return res;
}

let tmpURIError: URIError = new URIError('tmpURI error example');
function areURIErrorEqual(error: URIError) {
    let res: boolean = tmpURIError === error;
    tmpURIError = error;
    return res;
}