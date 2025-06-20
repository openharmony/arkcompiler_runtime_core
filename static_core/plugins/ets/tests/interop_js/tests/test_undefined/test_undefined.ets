/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

import { etsUndefinedProduce, etsUndefinedConsume, etsNullProduce, etsNullConsume } from "js_code"

function TestPassUndefinedToEts(): boolean {
    let a: Any = etsUndefinedProduce()
    if (a as JSValue !== JSRuntime.getUndefined()) {
        return false;
    }
    if (a == null) {
        return false;
    }
    return true;
}

function TestPassUndefinedFromEts(): boolean {
    let mUndefined: Any = undefined;
    return etsUndefinedConsume(mUndefined) as boolean;
}

function TestPassNullToEts(): boolean {
    if (etsNullProduce() as Any !== null) {
        return false;
    }
    if (etsNullProduce() as Any === undefined) {
        return false;
    }
    return true;
}

function TestPassNullFromEts(): boolean {
    let mNull: Any = null;
    return etsNullConsume(mNull) as boolean;
}
