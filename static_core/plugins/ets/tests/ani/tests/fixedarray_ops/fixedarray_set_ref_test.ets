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

function GetArray(): FixedArray<(String | null)> {
    let a: FixedArray<(String | null)> = [null, null , new String("Hello World!")];
    return a;
}

function GetNewString1(): String {
    return new String("New String 1!");
}

function GetNewString2(): String {
    return new String("New String 2!");
}

function CheckArray(array: FixedArray<(String | null)>): boolean {
    return array[0] == "New String 1!" && array[1] == null && array[2] == "New String 2!";
}

function GetObject(): Object {
    return new Object();
}
