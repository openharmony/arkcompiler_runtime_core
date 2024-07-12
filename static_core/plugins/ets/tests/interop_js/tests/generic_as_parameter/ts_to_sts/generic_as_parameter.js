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
exports.DataClass = exports.userClassInstance = exports.UserClass = 
exports.GTypeClass = exports.GInterfaceClass = 
exports.extendGenericLiteral = exports.extendGenericTuple = 
exports.extendGenericUnion = exports.extendGenericObj = 
exports.extendGenericArr = exports.extendGenericBool = 
exports.extendGenericString = exports.extendGenericNumber = 
exports.genericTypeFunctionReturnAny = exports.tsLiteral = exports.tsTuple = 
exports.tsUnion = exports.tsObj = exports.tsArr = exports.tsBool = 
exports.tsString = exports.tsNumber = void 0;
exports.anyTypeParameter = anyTypeParameter;
exports.anyTypeParameterExplicitCallFromTsInt = anyTypeParameterExplicitCallFromTsInt;
exports.anyTypeParameterExplicitCallFromTsString = anyTypeParameterExplicitCallFromTsString;
exports.anyTypeParameterExplicitCallFromTsBool = anyTypeParameterExplicitCallFromTsBool;
exports.anyTypeParameterExplicitCallFromTsArr = anyTypeParameterExplicitCallFromTsArr;
exports.anyTypeParameterExplicitCallFromTsObj = anyTypeParameterExplicitCallFromTsObj;
exports.anyTypeParameterExplicitCallFromTsUnion = anyTypeParameterExplicitCallFromTsUnion;
exports.anyTypeParameterExplicitCallFromTsTuple = anyTypeParameterExplicitCallFromTsTuple;
exports.anyTypeParameterExplicitCallFromTsLiteral = anyTypeParameterExplicitCallFromTsLiteral;
exports.genericTypeFunctionExplicitCallFromTsInt = genericTypeFunctionExplicitCallFromTsInt;
exports.genericTypeFunctionExplicitCallFromTsString = genericTypeFunctionExplicitCallFromTsString;
exports.genericTypeFunctionExplicitCallFromTsBool = genericTypeFunctionExplicitCallFromTsBool;
exports.genericTypeFunctionExplicitCallFromTsArr = genericTypeFunctionExplicitCallFromTsArr;
exports.genericTypeFunctionExplicitCallFromTsObj = genericTypeFunctionExplicitCallFromTsObj;
exports.genericTypeFunctionExplicitCallFromTsUnion = genericTypeFunctionExplicitCallFromTsUnion;
exports.genericTypeFunctionExplicitCallFromTsTuple = genericTypeFunctionExplicitCallFromTsTuple;
exports.genericTypeFunctionExplicitCallFromTsLiteral = genericTypeFunctionExplicitCallFromTsLiteral;
exports.genericExtendInterface = genericExtendInterface;
exports.genericExtendType = genericExtendType;
exports.tupleGeneric = tupleGeneric;
exports.collectGeneric = collectGeneric;
exports.createClassFromTs = createClassFromTs;
exports.genericDefaultInt = genericDefaultInt;
exports.genericDefaultString = genericDefaultString;
exports.genericDefaultBool = genericDefaultBool;
exports.genericDefaultArr = genericDefaultArr;
exports.genericDefaultObj = genericDefaultObj;
exports.genericDefaultUnion = genericDefaultUnion;
exports.genericDefaultTuple = genericDefaultTuple;
exports.genericDefaultLiteral = genericDefaultLiteral;
exports.genericDefaultIntCallFromTs = genericDefaultIntCallFromTs;
exports.genericDefaultStringCallFromTs = genericDefaultStringCallFromTs;
exports.genericDefaultBoolCallFromTs = genericDefaultBoolCallFromTs;
exports.genericDefaultArrCallFromTs = genericDefaultArrCallFromTs;
exports.genericDefaultObjCallFromTs = genericDefaultObjCallFromTs;
exports.genericDefaultUnionCallFromTs = genericDefaultUnionCallFromTs;
exports.genericDefaultTupleCallFromTs = genericDefaultTupleCallFromTs;
exports.genericDefaultLiteralCallFromTs = genericDefaultLiteralCallFromTs;
exports.genericExtendClass = genericExtendClass;
exports.tsNumber = 1;
exports.tsString = 'string';
exports.tsBool = true;
exports.tsArr = [exports.tsNumber];
exports.tsObj = {};
exports.tsUnion = exports.tsNumber;
exports.tsTuple = [exports.tsNumber, exports.tsBool];
exports.tsLiteral = exports.tsNumber;
function anyTypeParameter(arg) {
    return arg;
}
function anyTypeParameterExplicitCallFromTsInt() {
    return anyTypeParameter(exports.tsNumber);
}
function anyTypeParameterExplicitCallFromTsString() {
    return anyTypeParameter(exports.tsString);
}
function anyTypeParameterExplicitCallFromTsBool() {
    return anyTypeParameter(exports.tsBool);
}
function anyTypeParameterExplicitCallFromTsArr() {
    return anyTypeParameter(exports.tsArr);
}
function anyTypeParameterExplicitCallFromTsObj() {
    return anyTypeParameter(exports.tsObj);
}
function anyTypeParameterExplicitCallFromTsUnion() {
    return anyTypeParameter(exports.tsUnion);
}
function anyTypeParameterExplicitCallFromTsTuple() {
    return anyTypeParameter(exports.tsTuple);
}
function anyTypeParameterExplicitCallFromTsLiteral() {
    return anyTypeParameter(exports.tsLiteral);
}
let genericTypeFunctionReturnAny = function (arg) {
    return arg;
};
exports.genericTypeFunctionReturnAny = genericTypeFunctionReturnAny;
function genericTypeFunctionExplicitCallFromTsInt() {
    return (0, exports.genericTypeFunctionReturnAny)(exports.tsNumber);
}
function genericTypeFunctionExplicitCallFromTsString() {
    return (0, exports.genericTypeFunctionReturnAny)(exports.tsString);
}
function genericTypeFunctionExplicitCallFromTsBool() {
    return (0, exports.genericTypeFunctionReturnAny)(exports.tsBool);
}
function genericTypeFunctionExplicitCallFromTsArr() {
    return (0, exports.genericTypeFunctionReturnAny)(exports.tsArr);
}
function genericTypeFunctionExplicitCallFromTsObj() {
    return (0, exports.genericTypeFunctionReturnAny)(exports.tsObj);
}
function genericTypeFunctionExplicitCallFromTsUnion() {
    return (0, exports.genericTypeFunctionReturnAny)(exports.tsUnion);
}
function genericTypeFunctionExplicitCallFromTsTuple() {
    return (0, exports.genericTypeFunctionReturnAny)(exports.tsTuple);
}
function genericTypeFunctionExplicitCallFromTsLiteral() {
    return (0, exports.genericTypeFunctionReturnAny)(exports.tsLiteral);
}
let extendGenericNumber = function (arg) { return arg; };
exports.extendGenericNumber = extendGenericNumber;
let extendGenericString = function (arg) { return arg; };
exports.extendGenericString = extendGenericString;
let extendGenericBool = function (arg) { return arg; };
exports.extendGenericBool = extendGenericBool;
let extendGenericArr = function (arg) { return arg; };
exports.extendGenericArr = extendGenericArr;
let extendGenericObj = function (arg) { return arg; };
exports.extendGenericObj = extendGenericObj;
let extendGenericUnion = function (arg) { return arg; };
exports.extendGenericUnion = extendGenericUnion;
let extendGenericTuple = function (arg) { return arg; };
exports.extendGenericTuple = extendGenericTuple;
let extendGenericLiteral = function (arg) { return arg; };
exports.extendGenericLiteral = extendGenericLiteral;
let GInterfaceClass = /** @class */ (function () {
    function gInterfaceClass() {
        this.data = 'string';
    }
    return gInterfaceClass;
}());
exports.GInterfaceClass = GInterfaceClass;
let GTypeClass = /** @class */ (function () {
    function gTypeClass() {
        this.data = 'string';
    }
    return gTypeClass;
}());
exports.GTypeClass = GTypeClass;
function genericExtendInterface(arg) {
    return arg;
}
function genericExtendType(arg) {
    return arg;
}
function tupleGeneric(arg) {
    return [arg[1], arg[0]];
}
function collectGeneric(...arg) {
    let args = [];
    for (let _i = 0; _i < arg.length; _i++) {
        args[_i] = arg[_i];
    }
    return args;
}
let UserClass = /** @class */ (function () {
    function userClass(name, age) {
        this.name = name;
        this.age = age;
    }
    userClass.prototype.getName = function () {
        return this.name;
    };
    userClass.prototype.getAge = function () {
        return this.age;
    };
    return userClass;
}());
exports.UserClass = UserClass;
function createClassFromTs() {
    return new UserClass(exports.tsString, exports.tsNumber);
}
exports.userClassInstance = new UserClass(exports.tsString, exports.tsNumber);
function genericDefaultInt(arg) {
    return arg;
}
function genericDefaultString(arg) {
    return arg;
}
function genericDefaultBool(arg) {
    return arg;
}
function genericDefaultArr(arg) {
    return arg;
}
function genericDefaultObj(arg) {
    return arg;
}
function genericDefaultUnion(arg) {
    return arg;
}
function genericDefaultTuple(arg) {
    return arg;
}
function genericDefaultLiteral(arg) {
    return arg;
}
function genericDefaultIntCallFromTs() {
    return genericDefaultInt(exports.tsNumber);
}
function genericDefaultStringCallFromTs() {
    return genericDefaultString(exports.tsString);
}
function genericDefaultBoolCallFromTs() {
    return genericDefaultBool(exports.tsBool);
}
function genericDefaultArrCallFromTs() {
    return genericDefaultArr(exports.tsArr);
}
function genericDefaultObjCallFromTs() {
    return genericDefaultObj(exports.tsObj);
}
function genericDefaultUnionCallFromTs() {
    return genericDefaultUnion(exports.tsUnion);
}
function genericDefaultTupleCallFromTs() {
    return genericDefaultTuple(exports.tsTuple);
}
function genericDefaultLiteralCallFromTs() {
    return genericDefaultLiteral(exports.tsLiteral);
}
let DataClass = /** @class */ (function () {
    function dataClass() {
        this.data = 'string';
    }
    return dataClass;
}());
exports.DataClass = DataClass;
function genericExtendClass(arg) {
    return arg;
}
