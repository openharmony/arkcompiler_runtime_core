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

import { tsIntString } from 'main_js';

function checkConversionStringToInt(): boolean {
    const val = Double.toInt(parseInt(tsIntString));

    return typeof val == 'number';
}

function checkConversionStringToNumber(): boolean {
    const val = parseInt(tsIntString);

    return typeof val == 'number';
}

function checkConversionStringToFloat(): boolean {
    const val = Double.toFloat(parseInt(tsIntString));

    return typeof val == 'number';
}

function checkConversionStringToByte(): boolean {
    const val = Double.toByte(parseInt(tsIntString));

    return typeof val == 'number';
}

function checkConversionStringToShort(): boolean {
    const val = Double.toShort(parseInt(tsIntString));

    return typeof val == 'number';
}

function checkConversionStringToLong(): boolean {
    const val = Double.toLong(parseInt(tsIntString));

    return typeof val == 'number';
}

function checkConversionStringToDouble(): boolean {
    const val = parseInt(tsIntString);

    return typeof val == 'number';
}

function checkConversionStringToChar(): boolean {
    const val = Int.toChar(Double.toInt(parseInt(tsIntString)));

    return typeof val == 'number';
}
