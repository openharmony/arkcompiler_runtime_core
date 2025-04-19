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
    tsInt,
    tsBigInt,
    tsBinary,
    tsHexadecimal,
    tsIntString,
    tsByte,
    tsFloat,
    systemHexadecimal,
    systemBinary,
    tsExponential,
} from 'main_js';

function checkConversionIntToArrayInt(): boolean {
    const val: FixedArray<int> = [tsInt as int];

    return typeof val[0] == 'number';
}

function checkConversionIntToArrayNumber(): boolean {
    const val: FixedArray<number> = [tsInt as number];

    return typeof val[0] == 'number';
}

function checkConversionIntToArrayFloat(): boolean {
    const val: FixedArray<float> = [tsInt as float];

    return typeof val[0] == 'number';
}

function checkConversionIntToArrayByte(): boolean {
    const val: FixedArray<byte> = [tsInt as byte];

    return typeof val[0] == 'number';
}

function checkConversionIntToArrayShort(): boolean {
    const val: FixedArray<short> = [tsInt as short];

    return typeof val[0] == 'number';
}

function checkConversionIntToArrayLong(): boolean {
    const val: FixedArray<long> = [tsInt as long];

    return typeof val[0] == 'number';
}

function checkConversionIntToArrayDouble(): boolean {
    const val: FixedArray<double> = [tsInt as double];

    return typeof val[0] == 'number';
}

function checkConversionIntToArrayChar(): boolean {
    const val: FixedArray<char> = [tsInt as char];

    return typeof val[0] == 'number';
}

function checkConversionBinaryToArrayInt(): boolean {
    const val: FixedArray<int> = [parseInt(tsBinary, systemBinary as int) as int];

    return typeof val[0] == 'number';
}

function checkConversionBinaryToArrayNumber(): boolean {
    const val: FixedArray<number> = [parseInt(tsBinary, systemBinary as int) as number];

    return typeof val[0] == 'number';
}

function checkConversionBinaryToArrayFloat(): boolean {
    const val: FixedArray<float> = [parseInt(tsBinary, systemBinary as int) as float];

    return typeof val[0] == 'number';
}

function checkConversionBinaryToArrayByte(): boolean {
    const val: FixedArray<byte> = [parseInt(tsBinary, systemBinary as int) as byte];

    return typeof val[0] == 'number';
}

function checkConversionBinaryToArrayShort(): boolean {
    const val: FixedArray<short> = [parseInt(tsBinary, systemBinary as int) as short];

    return typeof val[0] == 'number';
}

function checkConversionBinaryToArrayLong(): boolean {
    const val: FixedArray<long> = [parseInt(tsBinary, systemBinary as int) as long];

    return typeof val[0] == 'number';
}

function checkConversionBinaryToArrayDouble(): boolean {
    const val: FixedArray<double> = [parseInt(tsBinary, systemBinary as int) as double];

    return typeof val[0] == 'number';
}

function checkConversionBinaryToArrayChar(): boolean {
    const val: FixedArray<char> = [parseInt(tsBinary, systemBinary as int) as char];

    return typeof val[0] == 'number';
}

function checkConversionHexadecimalToArrayInt(): boolean {
    const val: FixedArray<int> = [parseInt(tsHexadecimal, systemHexadecimal as int) as int];

    return typeof val[0] == 'number';
}

function checkConversionHexadecimalToArrayNumber(): boolean {
    const val: FixedArray<number> = [parseInt(tsHexadecimal, systemHexadecimal as int) as number];

    return typeof val[0] == 'number';
}

function checkConversionHexadecimalToArrayFloat(): boolean {
    const val: FixedArray<float> = [parseInt(tsHexadecimal, systemHexadecimal as int) as float];

    return typeof val[0] == 'number';
}

function checkConversionHexadecimalToArrayByte(): boolean {
    const val: FixedArray<byte> = [parseInt(tsHexadecimal, systemHexadecimal as int) as byte];

    return typeof val[0] == 'number';
}

function checkConversionHexadecimalToArrayShort(): boolean {
    const val: FixedArray<short> = [parseInt(tsHexadecimal, systemHexadecimal as int) as short];

    return typeof val[0] == 'number';
}

function checkConversionHexadecimalToArrayLong(): boolean {
    const val: FixedArray<long> = [parseInt(tsHexadecimal, systemHexadecimal as int) as long];

    return typeof val[0] == 'number';
}

function checkConversionHexadecimalToArrayDouble(): boolean {
    const val: FixedArray<double> = [parseInt(tsHexadecimal, systemHexadecimal as int) as double];

    return typeof val[0] == 'number';
}

function checkConversionHexadecimalToArrayChar(): boolean {
    const val: FixedArray<char> = [parseInt(tsHexadecimal, systemHexadecimal as int) as char];

    return typeof val[0] == 'number';
}

function checkConversionIntStringToArrayInt(): boolean {
    const val: FixedArray<int> = [parseInt(tsIntString) as int];

    return typeof val[0] == 'number';
}

function checkConversionIntStringToArrayNumber(): boolean {
    const val: FixedArray<number> = [parseInt(tsIntString) as number];

    return typeof val[0] == 'number';
}

function checkConversionIntStringToArrayFloat(): boolean {
    const val: FixedArray<float> = [parseInt(tsIntString) as float];

    return typeof val[0] == 'number';
}

function checkConversionIntStringToArrayByte(): boolean {
    const val: FixedArray<byte> = [parseInt(tsIntString) as byte];

    return typeof val[0] == 'number';
}

function checkConversionIntStringToArrayShort(): boolean {
    const val: FixedArray<short> = [parseInt(tsIntString) as short];

    return typeof val[0] == 'number';
}

function checkConversionIntStringToArrayLong(): boolean {
    const val: FixedArray<long> = [parseInt(tsIntString) as long];

    return typeof val[0] == 'number';
}

function checkConversionIntStringToArrayDouble(): boolean {
    const val: FixedArray<double> = [parseInt(tsIntString) as double];

    return typeof val[0] == 'number';
}

function checkConversionIntStringToArrayChar(): boolean {
    const val: FixedArray<char> = [parseInt(tsIntString) as char];

    return typeof val[0] == 'number';
}

function checkConversionFloatToArrayInt(): boolean {
    const val: FixedArray<int> = [tsFloat as int];

    return typeof val[0] == 'number';
}

function checkConversionFloatToArrayNumber(): boolean {
    const val: FixedArray<number> = [tsFloat as number];

    return typeof val[0] == 'number';
}

function checkConversionFloatToArrayFloat(): boolean {
    const val: FixedArray<float> = [tsFloat as float];

    return typeof val[0] == 'number';
}

function checkConversionFloatToArrayByte(): boolean {
    const val: FixedArray<byte> = [tsFloat as byte];

    return typeof val[0] == 'number';
}

function checkConversionFloatToArrayShort(): boolean {
    const val: FixedArray<short> = [tsFloat as short];

    return typeof val[0] == 'number';
}

function checkConversionFloatToArrayLong(): boolean {
    const val: FixedArray<long> = [tsFloat as long];

    return typeof val[0] == 'number';
}

function checkConversionFloatToArrayDouble(): boolean {
    const val: FixedArray<double> = [tsFloat as double];

    return typeof val[0] == 'number';
}

function checkConversionFloatToArrayChar(): boolean {
    const val: FixedArray<char> = [tsFloat as char];

    return typeof val[0] == 'number';
}

function checkConversionByteToArrayInt(): boolean {
    const val: FixedArray<int> = [tsByte.getUint8(0) as int];

    return typeof val[0] == 'number';
}

function checkConversionByteToArrayNumber(): boolean {
    const val: FixedArray<number> = [tsByte.getUint8(0) as number];

    return typeof val[0] == 'number';
}

function checkConversionByteToArrayFloat(): boolean {
    const val: FixedArray<float> = [tsByte.getUint8(0) as float];

    return typeof val[0] == 'number';
}

function checkConversionByteToArrayByte(): boolean {
    const val: FixedArray<byte> = [tsByte.getUint8(0) as byte];

    return typeof val[0] == 'number';
}

function checkConversionByteToArrayShort(): boolean {
    const val: FixedArray<short> = [tsByte.getUint8(0) as short];

    return typeof val[0] == 'number';
}

function checkConversionByteToArrayLong(): boolean {
    const val: FixedArray<long> = [tsByte.getUint8(0) as long];

    return typeof val[0] == 'number';
}

function checkConversionByteToArrayDouble(): boolean {
    const val: FixedArray<double> = [tsByte.getUint8(0) as double];

    return typeof val[0] == 'number';
}

function checkConversionByteToArrayChar(): boolean {
    const val: FixedArray<char> = [tsByte.getUint8(0) as char];

    return typeof val[0] == 'number';
}

function checkConversionBigIntToArrayInt(): boolean {
    const val: FixedArray<int> = [Number(tsBigInt) as int];

    return typeof val[0] == 'number';
}

function checkConversionBigIntToArrayNumber(): boolean {
    const val: FixedArray<number> = [Number(tsBigInt) as number];

    return typeof val[0] == 'number';
}

function checkConversionBigIntToArrayFloat(): boolean {
    const val: FixedArray<float> = [Number(tsBigInt) as float];

    return typeof val[0] == 'number';
}

function checkConversionBigIntToArrayByte(): boolean {
    const val: FixedArray<byte> = [Number(tsBigInt) as byte];

    return typeof val[0] == 'number';
}

function checkConversionBigIntToArrayShort(): boolean {
    const val: FixedArray<short> = [Number(tsBigInt) as short];

    return typeof val[0] == 'number';
}

function checkConversionBigIntToArrayLong(): boolean {
    const val: FixedArray<long> = [Number(tsBigInt) as long];

    return typeof val[0] == 'number';
}

function checkConversionBigIntToArrayDouble(): boolean {
    const val: FixedArray<double> = [Number(tsBigInt) as double];

    return typeof val[0] == 'number';
}

function checkConversionBigIntToArrayChar(): boolean {
    const val: FixedArray<char> = [Number(tsBigInt) as char];

    return typeof val[0] == 'number';
}

function checkConversionExponentialToArrayInt(): boolean {
    const val: FixedArray<int> = [parseInt(tsExponential) as int];

    return typeof val[0] == 'number';
}

function checkConversionExponentialToArrayNumber(): boolean {
    const val: FixedArray<number> = [parseInt(tsExponential) as number];

    return typeof val[0] == 'number';
}

function checkConversionExponentialToArrayFloat(): boolean {
    const val: FixedArray<float> = [parseInt(tsExponential) as float];

    return typeof val[0] == 'number';
}

function checkConversionExponentialToArrayByte(): boolean {
    const val: FixedArray<byte> = [parseInt(tsExponential) as byte];

    return typeof val[0] == 'number';
}

function checkConversionExponentialToArrayShort(): boolean {
    const val: FixedArray<short> = [parseInt(tsExponential) as short];

    return typeof val[0] == 'number';
}

function checkConversionExponentialToArrayLong(): boolean {
    const val: FixedArray<long> = [parseInt(tsExponential) as long];

    return typeof val[0] == 'number';
}

function checkConversionExponentialToArrayDouble(): boolean {
    const val: FixedArray<double> = [parseInt(tsExponential) as double];

    return typeof val[0] == 'number';
}

function checkConversionExponentialToArrayChar(): boolean {
    const val: FixedArray<char> = [parseInt(tsExponential) as char];

    return typeof val[0] == 'number';
}