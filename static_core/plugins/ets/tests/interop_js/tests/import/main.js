'use strict';
/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
Object.defineProperty(exports, '__esModule', { value: true });
exports.User = exports.tsReg = exports.tsDate = exports.tsFunction = 
exports.arr = exports.tsObject = exports.TsClass = exports.tsBigInt = 
exports.tsNewSymbol = exports.tsSymbol = exports.tsNull = 
exports.tsUndefined = exports.tsFalse = exports.tsTrue = 
exports.tsString = exports.tsNumber = void 0;
exports.increment = increment;
exports.concatString = concatString;
exports.changeTrue = changeTrue;
exports.changeFalse = changeFalse;
exports.changeUndefined = changeUndefined;
exports.changeNull = changeNull;
exports.changeSymbol = changeSymbol;
exports.changeBigInt = changeBigInt;
exports.changeTsClass = changeTsClass;
exports.changeObject = changeObject;
exports.changeArray = changeArray;
exports.changeFunction = changeFunction;
exports.changeDate = changeDate;
exports.changeRegExp = changeRegExp;
exports.default = defaultFu;
// Primitive types
exports.tsNumber = 1;
exports.tsString = 'Alice';
exports.tsTrue = true;
exports.tsFalse = false;
exports.tsNull = null;
exports.tsSymbol = Symbol('id');
exports.tsNewSymbol = Symbol('new');
exports.tsBigInt = 9007199254740991n;
function increment() {
    return exports.tsNumber++;
}
function concatString() {
    return exports.tsString += exports.tsString;
}
function changeTrue() {
    return exports.tsTrue = !exports.tsTrue;
}
function changeFalse() {
    return exports.tsFalse = !exports.tsFalse;
}
function changeUndefined() {
    return exports.tsUndefined = exports.tsNumber;
}
function changeNull() {
    return exports.tsNull = exports.tsNumber;
}
function changeSymbol() {
    return exports.tsSymbol = exports.tsNewSymbol;
}
function changeBigInt() {
    return exports.tsBigInt = exports.tsBigInt - exports.tsBigInt;
}
//Objects
let TsClass = /** @class */ (function () {
    function tsClass() {
        this.name = exports.tsString;
    }
    return tsClass;
}());
exports.TsClass = TsClass;
function changeTsClass(obj) {
    console.log(obj, 'TS LOG<<');
    obj.name = exports.tsString + exports.tsString;
}
exports.tsObject = {
    value: 'tsString',
};
function changeObject() {
    exports.tsObject.value = exports.tsString + exports.tsString;
}
exports.arr = [exports.tsNumber];
function changeArray() {
    exports.arr.push(exports.tsNumber);
}
let tsFunction = function () { return exports.tsNumber; };
exports.tsFunction = tsFunction;
function changeFunction() {
    exports.tsFunction = function () { return exports.tsNumber + exports.tsNumber; };
}
exports.tsDate = new Date();
function changeDate() {
    exports.tsDate.setFullYear(exports.tsDate.getFullYear() + 1);
}
;
exports.tsReg = new RegExp('abc');
function changeRegExp() {
    exports.tsReg = new RegExp('xyz');
}
;
function defaultFu() {
    return exports.tsNumber;
}
let User = /** @class */ (function () {
    function user() {
    }
    return user;
}());
exports.User = User;
