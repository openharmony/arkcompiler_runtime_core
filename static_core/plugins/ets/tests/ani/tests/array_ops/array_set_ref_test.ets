/*
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

function GetArray(): (String | null)[] {
    let a = [null, null , new String("Hello World!")];
    return a;
}

function GetEscompatArray(): Array<String | null> {
    let a : Array<String | null> = new Array<String | null>(3)
    a[0] = null
    a[1] = null
    a[2] = new String("Hello World!")
    return a;
}

function GetNewString1(): String {
    return new String("New String 1!");
}

function GetNewString2(): String {
    return new String("New String 2!");
}

function CheckArray(array: (String | null)[]): boolean {
    return array[0] == "New String 1!" && array[1] == null && array[2] == "New String 2!";
}

function CheckEscompatArray(array: Array<String | null>): boolean {
    return array[0] == "New String 1!" && array[1] == null && array[2] == "New String 2!";
}

function GetNumber(): Number {
    return new Number(42);
}
