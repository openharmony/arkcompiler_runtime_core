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
    GenericExtends,
    genericClassExtendsCallFromTs,
} from "main_js"

function checkClassExtendsInt(): boolean {
    return GenericExtends.get(tsInt) as int == tsInt as int;
}

function checkClassExtendsString(): boolean {
    return GenericExtends.get(tsString) as string == tsString as string;
}

function checkClassExtendsBool(): boolean {
    return GenericExtends.get(tsBool) as boolean == tsBool as boolean;
}

function checkClassExtendsArr(): boolean {
    const res = GenericExtends.get(tsArr);
    return res as Array<int> instanceof Array && res[0] as int == tsArr[0] as int;
}

function checkClassExtendsMethodCallFromTsInt(): boolean {
    return genericClassExtendsCallFromTs(tsInt) as int == tsInt as int;
}

function checkClassExtendsMethodCallFromTsString(): boolean {
    return genericClassExtendsCallFromTs(tsString) as string == tsString as string;
}

function checkClassExtendsMethodCallFromTsBool(): boolean {
    return genericClassExtendsCallFromTs(tsBool) as boolean == tsBool as boolean;
}

function checkClassExtendsMethodCallFromTsArr(): boolean {
    const res = genericClassExtendsCallFromTs(tsArr);
    return res as Array<int> instanceof Array && res[0] as int == tsArr[0] as int;
}
