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

import { tsInt } from 'main_js';

function checkConversionToInt(): boolean {
    const val = tsInt as int;
    
    return typeof val == 'number';
}

function checkConversionToNumber(): boolean {
    const val = tsInt as number;

    return typeof val == 'number';
}

function checkConversionToFloat(): boolean {
    const val = tsInt as float;

    return typeof val == 'number';
}

function checkConversionToByte(): boolean {
    const val = tsInt as byte;

    return typeof val == 'number';
}

function checkConversionToShort(): boolean {
    const val = tsInt as short;

    return typeof val == 'number';
}

function checkConversionToLong(): boolean {
    const val = tsInt as long;

    return typeof val == 'number';
}

function checkConversionToDouble(): boolean {
    const val = tsInt as double;

    return typeof val == 'number';
}

function checkConversionToChar(): boolean {
    const val = tsInt as char;

    return typeof val == 'number';
}
