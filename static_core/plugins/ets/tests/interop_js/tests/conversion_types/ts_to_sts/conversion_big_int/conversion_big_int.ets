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

import { tsBigInt, returnBigInt, returnBigIntObj } from 'main_js';

function checkTypeBigInt(): boolean {
    return typeof tsBigInt == 'object';
}

function checkConversionBigIntToInt(): boolean {
    const val = Double.toInt(Number(tsBigInt));

    return typeof val == 'number';
}

function checkConversionBigIntToNumber(): boolean {
    const val = Number(tsBigInt);

    return typeof val == 'number';
}

function checkConversionBigIntToFloat(): boolean {
    const val = Double.toFloat(Number(tsBigInt));

    return typeof val == 'number';
}

function checkConversionBigIntToByte(): boolean {
    const val = Double.toByte(Number(tsBigInt));

    return typeof val == 'number';
}

function checkConversionBigIntToShort(): boolean {
    const val = Double.toShort(Number(tsBigInt));

    return typeof val == 'number';
}

function checkConversionBigIntToLong(): boolean {
    const val = Double.toLong(Number(tsBigInt));

    return typeof val == 'number';
}

function checkConversionBigIntToDouble(): boolean {
    const val = Number(tsBigInt);

    return typeof val == 'number';
}

function checkConversionBigIntToChar(): boolean {
    const val = Int.toChar(Double.toInt(Number(tsBigInt)));

    return typeof val == 'number';
}

function checkReturnBigInt(): boolean {
    const val = returnBigInt() as bigint;

    return typeof val == 'bigint' && val == 1000000n;
}

function checkReturnBigIntObj(): boolean {
    const val = returnBigIntObj() as bigint;

    return typeof val == 'bigint' && val == 1n;
}
