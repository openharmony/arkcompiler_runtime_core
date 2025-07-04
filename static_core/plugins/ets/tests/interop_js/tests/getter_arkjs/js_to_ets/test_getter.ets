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

package getter.test;

function checkClassGetter(classObjWithGetter: JSValue, targetValue: JSValue) {
    let clsObj = ESValue.wrap(classObjWithGetter);
    let tarObj = ESValue.wrap(targetValue);
    return clsObj.getProperty('hello').toString() === tarObj.toString();
}

function checkLiteralObjGetter(literalObjWithGetter: JSValue, targetValue: JSValue) {
    let literalObj = ESValue.wrap(literalObjWithGetter);
    let tarObj = ESValue.wrap(targetValue);
    return literalObj.getProperty('hello').toString() == tarObj.toString();
}

function checkDeleteLiteralObjGetter(literalObjWithGetter: JSValue) {
    let literalObj = ESValue.wrap(literalObjWithGetter);
    return literalObj.getProperty('hello').toUndefined() == undefined;
}

function checkDefinePropertyGetter(literalObjWithGetter: JSValue, targetValue: JSValue) {
    let literalObj = ESValue.wrap(literalObjWithGetter);
    let tarObj = ESValue.wrap(targetValue);
    return literalObj.getProperty('hello').toString() == tarObj.toString();
}

function checkComputedPropertyGetter(literalObjWithGetter: JSValue, targetValue: JSValue) {
    let literalObj = ESValue.wrap(literalObjWithGetter);
    let tarObj = ESValue.wrap(targetValue);
    return literalObj.getProperty('hello').toString() == tarObj.toString();
}

