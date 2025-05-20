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

import { tsHexadecimal, systemHexadecimal } from 'main_js';

function checkConversionHexadecimalToInt(): boolean {
    const val = Double.toInt(parseInt(tsHexadecimal, systemHexadecimal as int));

    return typeof val == 'number';
}

function checkConversionHexadecimalToNumber(): boolean {
    const val = parseInt(tsHexadecimal, systemHexadecimal as int);

    return typeof val == 'number';
}

function checkConversionHexadecimalToFloat(): boolean {
    const val = Double.toFloat(parseInt(tsHexadecimal, systemHexadecimal as int));

    return typeof val == 'number';
}

function checkConversionHexadecimalToByte(): boolean {
    const val = Double.toByte(parseInt(tsHexadecimal, systemHexadecimal as int));

    return typeof val == 'number';
}

function checkConversionHexadecimalToShort(): boolean {
    const val = Double.toShort(parseInt(tsHexadecimal, systemHexadecimal as int));

    return typeof val == 'number';
}

function checkConversionHexadecimalToLong(): boolean {
    const val = Double.toLong(parseInt(tsHexadecimal, systemHexadecimal as int));

    return typeof val == 'number';
}

function checkConversionHexadecimalToDouble(): boolean {
    const val = parseInt(tsHexadecimal, systemHexadecimal as int);

    return typeof val == 'number';
}

function checkConversionHexadecimalToChar(): boolean {
    const val = Int.toChar(Double.toInt(parseInt(tsHexadecimal, systemHexadecimal as int)));

    return typeof val == 'number';
}
