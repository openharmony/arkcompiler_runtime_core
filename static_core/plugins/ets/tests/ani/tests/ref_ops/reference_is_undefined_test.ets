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

function GetNull(): null {
    return null
}

function GetUndefined(): undefined {
    return undefined
}

function getObject(): Object {
    return "Hello World!"
}

native function isUndefined(arg: Object | null | undefined): boolean

export function checkUndefinedReference(): boolean {
    return isUndefined(undefined) && !isUndefined(null) && !isUndefined("abc")
}

native function getUndefinedFromNative(): undefined
export function checkUndefinedFromNative(): boolean {
    return getUndefinedFromNative() === undefined;
}
