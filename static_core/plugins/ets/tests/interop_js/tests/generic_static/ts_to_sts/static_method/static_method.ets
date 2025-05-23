/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

import {
    tsInt,
    tsString,
    tsBool,
    tsArr,
    GenericStatic,
    genericStaticMethodCallFromTs,
} from "main_js"

function checkGenericStaticInt(): boolean {
    return GenericStatic.get(tsInt) as int == tsInt as int;
}

function checkGenericStaticString(): boolean {
    return GenericStatic.get(tsString) as string == tsString as string;
}

function checkGenericStaticBool(): boolean {
    return GenericStatic.get(tsBool) as boolean == tsBool as boolean;
}

function checkGenericStaticArr(): boolean {
    const res = GenericStatic.get(tsArr);
    return res as Array<int> instanceof Array && res[0] as int == tsArr[0] as int;
}

function checkGenericStaticMethodCallFromTsInt(): boolean {
    return genericStaticMethodCallFromTs(tsInt) as int == tsInt as int;
}

function checkGenericStaticMethodCallFromTsString(): boolean {
    return genericStaticMethodCallFromTs(tsString) as string == tsString as string;
}

function checkGenericStaticMethodCallFromTsBool(): boolean {
    return genericStaticMethodCallFromTs(tsBool) as boolean == tsBool as boolean;
}

function checkGenericStaticMethodCallFromTsArr(): boolean {
    const res = genericStaticMethodCallFromTs(tsArr);
    return res as Array<int> instanceof Array && res[0] as int == tsArr[0] as int;
}
