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

import { tsByte } from 'main_js';

function checkConversionByteToInt(): boolean {
    const val = tsByte.getUint8(0) as int;

    return typeof val == 'number';
}

function checkConversionByteToNumber(): boolean {
    const val = tsByte.getUint8(0) as number;

    return typeof val == 'number';
}

function checkConversionByteToFloat(): boolean {
    const val = tsByte.getUint8(0) as float;

    return typeof val == 'number';
}

function checkConversionByteToByte(): boolean {
    const val = tsByte.getUint8(0) as byte;

    return typeof val == 'number';
}

function checkConversionByteToShort(): boolean {
    const val = tsByte.getUint8(0) as short;

    return typeof val == 'number';
}

function checkConversionByteToLong(): boolean {
    const val = tsByte.getUint8(0) as long;

    return typeof val == 'number';
}

function checkConversionByteToDouble(): boolean {
    const val = tsByte.getUint8(0) as double;

    return typeof val == 'number';
}

function checkConversionByteToChar(): boolean {
    const val = tsByte.getUint8(0) as char;

    return typeof val == 'number';
}
