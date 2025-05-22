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

import { tsBinary, systemBinary } from 'main_js';

function checkConversionBinaryToInt(): boolean {
    const val = Double.toInt(parseInt(tsBinary, systemBinary as int));

    return typeof val == 'number';
}

function checkConversionBinaryToNumber(): boolean {
    const val = parseInt(tsBinary, systemBinary as int);

    return typeof val == 'number';
}

function checkConversionBinaryToFloat(): boolean {
    const val = Double.toFloat(parseInt(tsBinary, systemBinary as int));

    return typeof val == 'number';
}

function checkConversionBinaryToByte(): boolean {
    const val = Double.toByte(parseInt(tsBinary, systemBinary as int));

    return typeof val == 'number';
}

function checkConversionBinaryToShort(): boolean {
    const val = Double.toShort(parseInt(tsBinary, systemBinary as int));

    return typeof val == 'number';
}

function checkConversionBinaryToLong(): boolean {
    const val = Double.toLong(parseInt(tsBinary, systemBinary as int));

    return typeof val == 'number';
}

function checkConversionBinaryToDouble(): boolean {
    const val = parseInt(tsBinary, systemBinary as int);

    return typeof val == 'number';
}

function checkConversionBinaryToChar(): boolean {
    const val = Int.toChar(Double.toInt(parseInt(tsBinary, systemBinary as int)));

    return typeof val == 'number';
}
