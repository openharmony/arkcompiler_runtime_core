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

import { tsFloat } from 'main_js';

function checkConversionFloatToInt(): boolean {
    const val = tsFloat as int;

    return typeof val == 'number';
}

function checkConversionFloatToNumber(): boolean {
    const val = tsFloat as number;

    return typeof val == 'number';
}

function checkConversionFloatToFloat(): boolean {
    const val = tsFloat as float;

    return typeof val == 'number';
}

function checkConversionFloatToByte(): boolean {
    const val = tsFloat as byte;

    return typeof val == 'number';
}

function checkConversionFloatToShort(): boolean {
    const val = tsFloat as short;

    return typeof val == 'number';
}

function checkConversionFloatToLong(): boolean {
    const val = tsFloat as long;

    return typeof val == 'number';
}

function checkConversionFloatToDouble(): boolean {
    const val = tsFloat as double;

    return typeof val == 'number';
}

function checkConversionFloatToChar(): boolean {
    const val = tsFloat as char;

    return typeof val == 'number';
}
