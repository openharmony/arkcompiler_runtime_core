/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

// Numbers
export let jsNumber = 123;
export let jsByte = 123;
export let jsShort = 123;
export let jsInt = 123;
export let jsLong = 123;
export let jsFloat = 12.34;
export let jsDouble = 12.34;

// Special
export let jsBoolean = true;
export let jsNull = null;
export let jsUndefined = undefined;
export let jsBigint = 1234567890123456789012345678901234567890n;

// String
export let jsString = 'Test';

// Array
export let jsArray = [1, 2, 3, 4, 5];

// Map
class Map {
    [key];
};

export let jsNumberMap = new Map();
jsNumberMap[0] = 0xcafe;
jsNumberMap[1] = 0xbabe;

export let jsStringMap = new Map();
jsStringMap['one'] = 0xcafe;
jsStringMap['two'] = 0xbabe;

class ValueHolder {
    jsNumber = 123;
    jsByte = 123;
    jsShort = 123;
    jsInt = 123;
    jsLong = 123;
    jsFloat = 12.34;
    jsDouble = 12.34;

    // Special
    jsBoolean = true;
    jsNull = null;
    jsUndefined = undefined;
    jsBigint = 1234567890123456789012345678901234567890n;
    jsString = 'Test';
    jsArray = [1, 2, 3, 4, 5];
}

export let jsValueHolder = new ValueHolder();

// Object
class Person {
    name = 'string';
    constructor() {
    }
}
export let jsObj = new Person();
export let jsObjCopy = jsObj;
export let jsObjOther = Object.assign({}, jsObj);
