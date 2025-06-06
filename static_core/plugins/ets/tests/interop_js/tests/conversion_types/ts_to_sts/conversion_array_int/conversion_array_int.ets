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
    const val: int[] = [tsInt as int];

    return typeof val[0] == 'number';
}

function checkConversionIntToArrayNumber(): boolean {
    const val: number[] = [tsInt as number];

    return typeof val[0] == 'number';
}

function checkConversionIntToArrayFloat(): boolean {
    const val: float[] = [tsInt as float];

    return typeof val[0] == 'number';
}

function checkConversionIntToArrayByte(): boolean {
    const val: byte[] = [tsInt as byte];

    return typeof val[0] == 'number';
}

function checkConversionIntToArrayShort(): boolean {
    const val: short[] = [tsInt as short];

    return typeof val[0] == 'number';
}

function checkConversionIntToArrayLong(): boolean {
    const val: long[] = [tsInt as long];

    return typeof val[0] == 'number';
}

function checkConversionIntToArrayDouble(): boolean {
    const val: double[] = [tsInt as double];

    return typeof val[0] == 'number';
}

function checkConversionIntToArrayChar(): boolean {
    const val: char[] = [tsInt as char];

    return typeof val[0] == 'number';
}

function checkConversionBinaryToArrayInt(): boolean {
    const val: int[] = [parseInt(tsBinary, systemBinary as int) as int];

    return typeof val[0] == 'number';
}

function checkConversionBinaryToArrayNumber(): boolean {
    const val: number[] = [parseInt(tsBinary, systemBinary as int) as number];

    return typeof val[0] == 'number';
}

function checkConversionBinaryToArrayFloat(): boolean {
    const val: float[] = [parseInt(tsBinary, systemBinary as int) as float];

    return typeof val[0] == 'number';
}

function checkConversionBinaryToArrayByte(): boolean {
    const val: byte[] = [parseInt(tsBinary, systemBinary as int) as byte];

    return typeof val[0] == 'number';
}

function checkConversionBinaryToArrayShort(): boolean {
    const val: short[] = [parseInt(tsBinary, systemBinary as int) as short];

    return typeof val[0] == 'number';
}

function checkConversionBinaryToArrayLong(): boolean {
    const val: long[] = [parseInt(tsBinary, systemBinary as int) as long];

    return typeof val[0] == 'number';
}

function checkConversionBinaryToArrayDouble(): boolean {
    const val: double[] = [parseInt(tsBinary, systemBinary as int) as double];

    return typeof val[0] == 'number';
}

function checkConversionBinaryToArrayChar(): boolean {
    const val: char[] = [parseInt(tsBinary, systemBinary as int) as char];

    return typeof val[0] == 'number';
}

function checkConversionHexadecimalToArrayInt(): boolean {
    const val: int[] = [parseInt(tsHexadecimal, systemHexadecimal as int) as int];

    return typeof val[0] == 'number';
}

function checkConversionHexadecimalToArrayNumber(): boolean {
    const val: number[] = [parseInt(tsHexadecimal, systemHexadecimal as int) as number];

    return typeof val[0] == 'number';
}

function checkConversionHexadecimalToArrayFloat(): boolean {
    const val: float[] = [parseInt(tsHexadecimal, systemHexadecimal as int) as float];

    return typeof val[0] == 'number';
}

function checkConversionHexadecimalToArrayByte(): boolean {
    const val: byte[] = [parseInt(tsHexadecimal, systemHexadecimal as int) as byte];

    return typeof val[0] == 'number';
}

function checkConversionHexadecimalToArrayShort(): boolean {
    const val: short[] = [parseInt(tsHexadecimal, systemHexadecimal as int) as short];

    return typeof val[0] == 'number';
}

function checkConversionHexadecimalToArrayLong(): boolean {
    const val: long[] = [parseInt(tsHexadecimal, systemHexadecimal as int) as long];

    return typeof val[0] == 'number';
}

function checkConversionHexadecimalToArrayDouble(): boolean {
    const val: double[] = [parseInt(tsHexadecimal, systemHexadecimal as int) as double];

    return typeof val[0] == 'number';
}

function checkConversionHexadecimalToArrayChar(): boolean {
    const val: char[] = [parseInt(tsHexadecimal, systemHexadecimal as int) as char];

    return typeof val[0] == 'number';
}

function checkConversionIntStringToArrayInt(): boolean {
    const val: int[] = [parseInt(tsIntString) as int];

    return typeof val[0] == 'number';
}

function checkConversionIntStringToArrayNumber(): boolean {
    const val: number[] = [parseInt(tsIntString) as number];

    return typeof val[0] == 'number';
}

function checkConversionIntStringToArrayFloat(): boolean {
    const val: float[] = [parseInt(tsIntString) as float];

    return typeof val[0] == 'number';
}

function checkConversionIntStringToArrayByte(): boolean {
    const val: byte[] = [parseInt(tsIntString) as byte];

    return typeof val[0] == 'number';
}

function checkConversionIntStringToArrayShort(): boolean {
    const val: short[] = [parseInt(tsIntString) as short];

    return typeof val[0] == 'number';
}

function checkConversionIntStringToArrayLong(): boolean {
    const val: long[] = [parseInt(tsIntString) as long];

    return typeof val[0] == 'number';
}

function checkConversionIntStringToArrayDouble(): boolean {
    const val: double[] = [parseInt(tsIntString) as double];

    return typeof val[0] == 'number';
}

function checkConversionIntStringToArrayChar(): boolean {
    const val: char[] = [parseInt(tsIntString) as char];

    return typeof val[0] == 'number';
}

function checkConversionFloatToArrayInt(): boolean {
    const val: int[] = [tsFloat as int];

    return typeof val[0] == 'number';
}

function checkConversionFloatToArrayNumber(): boolean {
    const val: number[] = [tsFloat as number];

    return typeof val[0] == 'number';
}

function checkConversionFloatToArrayFloat(): boolean {
    const val: float[] = [tsFloat as float];

    return typeof val[0] == 'number';
}

function checkConversionFloatToArrayByte(): boolean {
    const val: byte[] = [tsFloat as byte];

    return typeof val[0] == 'number';
}

function checkConversionFloatToArrayShort(): boolean {
    const val: short[] = [tsFloat as short];

    return typeof val[0] == 'number';
}

function checkConversionFloatToArrayLong(): boolean {
    const val: long[] = [tsFloat as long];

    return typeof val[0] == 'number';
}

function checkConversionFloatToArrayDouble(): boolean {
    const val: double[] = [tsFloat as double];

    return typeof val[0] == 'number';
}

function checkConversionFloatToArrayChar(): boolean {
    const val: char[] = [tsFloat as char];

    return typeof val[0] == 'number';
}

function checkConversionByteToArrayInt(): boolean {
    const val: int[] = [tsByte.getUint8(0) as int];

    return typeof val[0] == 'number';
}

function checkConversionByteToArrayNumber(): boolean {
    const val: number[] = [tsByte.getUint8(0) as number];

    return typeof val[0] == 'number';
}

function checkConversionByteToArrayFloat(): boolean {
    const val: float[] = [tsByte.getUint8(0) as float];

    return typeof val[0] == 'number';
}

function checkConversionByteToArrayByte(): boolean {
    const val: byte[] = [tsByte.getUint8(0) as byte];

    return typeof val[0] == 'number';
}

function checkConversionByteToArrayShort(): boolean {
    const val: short[] = [tsByte.getUint8(0) as short];

    return typeof val[0] == 'number';
}

function checkConversionByteToArrayLong(): boolean {
    const val: long[] = [tsByte.getUint8(0) as long];

    return typeof val[0] == 'number';
}

function checkConversionByteToArrayDouble(): boolean {
    const val: double[] = [tsByte.getUint8(0) as double];

    return typeof val[0] == 'number';
}

function checkConversionByteToArrayChar(): boolean {
    const val: char[] = [tsByte.getUint8(0) as char];

    return typeof val[0] == 'number';
}

function checkConversionBigIntToArrayInt(): boolean {
    const val: int[] = [Number(tsBigInt) as int];

    return typeof val[0] == 'number';
}

function checkConversionBigIntToArrayNumber(): boolean {
    const val: number[] = [Number(tsBigInt) as number];

    return typeof val[0] == 'number';
}

function checkConversionBigIntToArrayFloat(): boolean {
    const val: float[] = [Number(tsBigInt) as float];

    return typeof val[0] == 'number';
}

function checkConversionBigIntToArrayByte(): boolean {
    const val: byte[] = [Number(tsBigInt) as byte];

    return typeof val[0] == 'number';
}

function checkConversionBigIntToArrayShort(): boolean {
    const val: short[] = [Number(tsBigInt) as short];

    return typeof val[0] == 'number';
}

function checkConversionBigIntToArrayLong(): boolean {
    const val: long[] = [Number(tsBigInt) as long];

    return typeof val[0] == 'number';
}

function checkConversionBigIntToArrayDouble(): boolean {
    const val: double[] = [Number(tsBigInt) as double];

    return typeof val[0] == 'number';
}

function checkConversionBigIntToArrayChar(): boolean {
    const val: char[] = [Number(tsBigInt) as char];

    return typeof val[0] == 'number';
}

function checkConversionExponentialToArrayInt(): boolean {
    const val: int[] = [parseInt(tsExponential) as int];

    return typeof val[0] == 'number';
}

function checkConversionExponentialToArrayNumber(): boolean {
    const val: number[] = [parseInt(tsExponential) as number];

    return typeof val[0] == 'number';
}

function checkConversionExponentialToArrayFloat(): boolean {
    const val: float[] = [parseInt(tsExponential) as float];

    return typeof val[0] == 'number';
}

function checkConversionExponentialToArrayByte(): boolean {
    const val: byte[] = [parseInt(tsExponential) as byte];

    return typeof val[0] == 'number';
}

function checkConversionExponentialToArrayShort(): boolean {
    const val: short[] = [parseInt(tsExponential) as short];

    return typeof val[0] == 'number';
}

function checkConversionExponentialToArrayLong(): boolean {
    const val: long[] = [parseInt(tsExponential) as long];

    return typeof val[0] == 'number';
}

function checkConversionExponentialToArrayDouble(): boolean {
    const val: double[] = [parseInt(tsExponential) as double];

    return typeof val[0] == 'number';
}

function checkConversionExponentialToArrayChar(): boolean {
    const val: char[] = [parseInt(tsExponential) as char];

    return typeof val[0] == 'number';
}