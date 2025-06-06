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
    tsNumber,
    increment,
    tsString,
    concatString,
    tsTrue,
    tsFalse,
    changeTrue,
    changeFalse,
    tsUndefined,
    changeUndefined,
    tsNull,
    changeNull,
    tsSymbol,
    changeSymbol,
    tsBigInt,
    changeBigInt
} from 'main_js';

function checkChangeNumber(): boolean {
    const oldValue = tsNumber;
    increment();

    return tsNumber as int > oldValue as int;
}

function checkChangeString(): boolean {
    const oldValue = tsString;
    concatString();

    return tsString as string != oldValue as string;
}

function checkChangeTrue(): boolean {
    const oldValue = tsTrue;
    changeTrue();

    return oldValue as boolean != tsTrue as boolean;
}

function checkChangeFalse(): boolean {
    changeFalse();

    return tsFalse as boolean;
}

function checkChangeUndefined() {
    changeUndefined();

    return tsUndefined as int == tsNumber as int;
}

function checkChangeNull() {
    changeNull();

    return tsNull as int == tsNumber as int;
}

function checkChangeSymbol(): boolean {
    const oldValue = tsSymbol;
    changeSymbol();

    return oldValue != tsSymbol;
}

function checkChangeBigInt(): boolean {
    const oldValue = tsBigInt;
    changeBigInt();

    return oldValue as BigInt != tsBigInt as BigInt;
}
