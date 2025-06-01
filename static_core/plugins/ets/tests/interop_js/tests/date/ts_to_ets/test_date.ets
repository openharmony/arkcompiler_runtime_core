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

const datestr = '2025-03-01T01:02:03.000Z';
const timestamp = 1609459200000;

function FunObject(val: Object) {
    return val instanceof Date;
}
 
function FunDateInstanceof(val:Date) {
    return val instanceof Date;
}

function FunDatetypeof(val:Date) {
    return typeof val === 'object';
}

function FunDategetFullYear(val:Date) {
    return val.getFullYear() === 2025;
}

function FunDategetMonth(val:Date) {
    return val.getMonth() === 3;
}

function FunDategetDate(val:Date) {
    return val.getDate() === 1;
}

function FunDategetDay(val:Date) {
    return val.getDay() === 6;
}

function FunDategetHours(val:Date) {
    return val.getHours() === 1;
}

function FunDategetMinutes(val:Date) {
    return val.getMinutes() === 2;
}

function FunDategetSeconds(val:Date) {
    return val.getSeconds() === 3;
}

function FunDategetMilliseconds(val:Date) {
    return val.getMilliseconds() === 0;
}

function FunDatetoISOString(val:Date) {
    return val.toISOString() === datestr;
}

function FunDategetTime(val:Date) {
    return val.getTime() === timestamp;
}
