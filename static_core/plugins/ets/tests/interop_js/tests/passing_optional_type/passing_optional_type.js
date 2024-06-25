/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
function fnWithAnyParams(name, surname) {
    return surname ? `${name}; ${surname}` : name;
}
function fnWithAnyParamsObject(obj) {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}
function fnWithLiteralParam(name, surname) {
    return surname ? `${name}; ${surname}` : name;
}
function fnWithLiteralObjectParam(obj) {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}
function fnWithExtraSetParam(name, surname) {
    return surname ? `${name}; ${surname}` : name;
}
function fnWithExtraSetObjectParam(obj) {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}
function fnWithSubSetParam(obj) {
    return obj.city ? `${obj.name}; ${obj.city}` : obj.name;
}
function fnWithUnionParam(id) {
    return id ? id : 'id not found';
}
function fnWithUnionObjectParam(obj) {
    return obj.id ? obj.id : 'id not found';
}
function fnWithSubSetReduseParam(obj) {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}
function fnWithSubSetPartialParam(obj) {
    return obj.surname ? `${obj.name}; ${obj.surname}` : obj.name;
}

module.exports = {
    fnWithAnyParams,
    fnWithAnyParamsObject,
    fnWithLiteralParam,
    fnWithLiteralObjectParam,
    fnWithExtraSetParam,
    fnWithExtraSetObjectParam,
    fnWithSubSetParam,
    fnWithUnionParam,
    fnWithUnionObjectParam,
    fnWithSubSetReduseParam,
    fnWithSubSetPartialParam,
};