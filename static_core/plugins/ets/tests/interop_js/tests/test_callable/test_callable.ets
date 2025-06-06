/*
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
    callNamedFunction,
    callAnonymousFunction,
    callArrowFunction,
    callConstructedFunction,
    callBoundFunction
} from 'js_code';

const FIRST_VALUE = 5
const SECONT_VALUE = 5
const EXP_SUM = 10

function TestNamedFunction(): boolean {
    const testVal = callNamedFunction(FIRST_VALUE, SECONT_VALUE) as number
    return testVal == EXP_SUM
}

function TestAnonymousFunction(): boolean {
    const testVal = callAnonymousFunction(FIRST_VALUE, SECONT_VALUE) as number
    return testVal == EXP_SUM
}

function TestArrowFunction(): boolean {
    const testVal = callArrowFunction(FIRST_VALUE, SECONT_VALUE) as number
    return testVal == EXP_SUM
}

function TestConstructedFunction(): boolean {
    const testVal = callConstructedFunction(FIRST_VALUE, SECONT_VALUE) as number
    return testVal == EXP_SUM
}

function TestCallBoundFunction(): boolean {
    const testVal = callBoundFunction(FIRST_VALUE, SECONT_VALUE) as number
    return testVal == EXP_SUM
}
