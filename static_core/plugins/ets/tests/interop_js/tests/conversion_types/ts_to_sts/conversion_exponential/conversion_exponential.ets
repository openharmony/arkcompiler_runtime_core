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

import { tsExponential } from 'main_js';

function checkConversionExponentialToInt(): boolean {
    const val = Double.toInt(parseInt(tsExponential));

    return typeof val == 'number';
}

function checkConversionExponentialToNumber(): boolean {
    const val = parseInt(tsExponential);

    return typeof val == 'number';
}

function checkConversionExponentialToFloat(): boolean {
    const val = Double.toFloat(parseInt(tsExponential));

    return typeof val == 'number';
}

function checkConversionExponentialToByte(): boolean {
    const val = Double.toByte(parseInt(tsExponential));

    return typeof val == 'number';
}

function checkConversionExponentialToShort(): boolean {
    const val = Double.toShort(parseInt(tsExponential));

    return typeof val == 'number';
}

function checkConversionExponentialToLong(): boolean {
    const val = Double.toLong(parseInt(tsExponential));

    return typeof val == 'number';
}

function checkConversionExponentialToDouble(): boolean {
    const val = parseInt(tsExponential);

    return typeof val == 'number';
}

function checkConversionExponentialToChar(): boolean {
    const val = Int.toChar(Double.toInt(parseInt(tsExponential)));

    return typeof val == 'number';
}
