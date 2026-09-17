/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

function testIU8OfInt() {
    checkIntArray(new ValueArray<int>(0), Int8Array.of());
    checkIntArray(new ValueArray<int>(0), Uint8Array.of());
    checkIntArray(new ValueArray<int>(0), Uint8ClampedArray.of());

    let arr = prepareTestData(127, 1);
    checkIntArray(arr, Int8Array.of(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
        80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106,
        107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127));
    checkIntArray(arr, Uint8Array.of(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
        80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106,
        107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127));
    checkIntArray(arr, Uint8ClampedArray.of(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
        22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
        50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77,
        78, 79,  80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104,
        105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127));

    checkIntArray([0, 1,  -1,  -1, 1,  -1, 0], Int8Array.of(0, 1, -1, 65535, -65535, 2147483647, -2147483648));
    checkIntArray([0, 1, 255, 255, 1, 255, 0], Uint8Array.of(0, 1, -1, 65535, -65535, 2147483647, -2147483648));
    checkIntArray([0, 1,   0, 255, 0, 255, 0], Uint8ClampedArray.of(0, 1, -1, 65535, -65535, 2147483647, -2147483648));

    checkIntArray([1, -1, -126, -127, -128, 127, 0, -1, 0, 12, -1, 126, 127, -128, -2, 0, 1, 16],
        Int8Array.of(1, -1, -126, -127, -128, -129, -256, -257, 0, 12, 255, 126, 127, 128, 254, 256, 257, 10000));
    checkIntArray([1, 255, 130, 129, 128, 127, 0, 255, 0, 12, 255, 126, 127, 128, 254, 0, 1, 16],
        Uint8Array.of(1, -1, -126, -127, -128, -129, -256, -257, 0, 12, 255, 126, 127, 128, 254, 256, 257, 10000));
    checkIntArray([1, 0, 0, 0, 0, 0, 0, 0, 0, 12, 255, 126, 127, 128, 254, 255, 255, 255],
        Uint8ClampedArray.of(1, -1, -126, -127, -128, -129, -256, -257, 0, 12, 255, 126, 127, 128, 254, 256, 257, 10000));
}

function testIU16OfInt() {
    checkIntArray(new ValueArray<int>(0), Int16Array.of());
    checkIntArray(new ValueArray<int>(0), Uint16Array.of());

    let arr = prepareTestData16(127, 1);
    checkIntArray(arr, makeInt16Array(127, 1));
    checkIntArray(arr, makeUint16Array(127, 1));

    arr = prepareTestData16(127, 128);
    checkIntArray(arr, makeInt16Array(127, 128));
    checkIntArray(arr, makeUint16Array(127, 128));

    checkIntArray([0, 1,    -1,    -1, 1,    -1, 0], Int16Array.of(0, 1, -1, 65535, -65535, 2147483647, -2147483648));
    checkIntArray([0, 1, 65535, 65535, 1, 65535, 0], Uint16Array.of(0, 1, -1, 65535, -65535, 2147483647, -2147483648));

    checkIntArray([-1, -32766, -32767, -32768, 32767, 1, 0, -1, 0, 12, 255, 32766, 32765, -32768, -32767, -2, -1, 0, -31072, -1, 0],
        Int16Array.of(-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12, 255, 32766, 32765, 32768, 32769,
            65534, 65535, 65536, 100000, 2147483647, -2147483648));
    checkIntArray([65535, 32770, 32769, 32768, 32767, 1, 0, 65535, 0, 12, 255, 32766, 32765, 32768, 32769, 65534, 65535,
            0, 34464, 65535, 0],
        Uint16Array.of(-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12, 255, 32766, 32765, 32768, 32769,
            65534, 65535, 65536, 100000, 2147483647, -2147483648));
}

function testIU32OfInt() {
    checkIntArray(new ValueArray<int>(0), Int32Array.of());
    checkIntArray(new ValueArray<int>(0), Uint32Array.of());

    let arr = prepareTestData32(127, 1);
    checkIntArray(arr, makeInt32Array(127, 1));
    checkIntArray(arr, makeUint32Array(127, 1));

    arr = prepareTestData32(127, 16536);
    checkIntArray(arr, makeInt32Array(127, 16536));
    checkIntArray(arr, makeUint32Array(127, 16536));

    checkIntArray([0, 1, -1, 65535, -65535, 2147483647, -2147483648],
        Int32Array.of(0, 1, -1, 65535, -65535, 2147483647, -2147483648));
    checkNumberArray([0.0, 1.0, 4294967295.0, 65535.0, 4294901761.0, 2147483647.0, 2147483648.0],
        Uint32Array.of(0, 1, -1, 65535, -65535, 2147483647, -2147483648));

    checkIntArray([-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12, 255, 32766, 32765, 32768, 32769,
            65534, 65535, 65536, 100000, 2147483647, -2147483648],
        Int32Array.of(-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12, 255, 32766, 32765, 32768, 32769,
            65534, 65535, 65536, 100000, 2147483647, -2147483648));
    checkNumberArray([4294967295.0, 4294934530.0, 4294934529.0, 4294934528.0, 4294934527.0, 4294901761.0,
            4294901760.0, 4294901759.0, 0.0, 12.0, 255.0, 32766.0, 32765.0, 32768.0, 32769.0, 65534.0, 65535.0,
            65536.0, 100000.0, 2147483647.0, 2147483648.0],
        Uint32Array.of(-1, -32766, -32767, -32768, -32769, -65535, -65536, -65537, 0, 12, 255, 32766, 32765, 32768, 32769,
            65534, 65535, 65536, 100000, 2147483647, -2147483648));
}

function testIU32OfNumber() {
    checkNumberArray([0.0, 1.0, 1.0, -1.0, 65535.0, -65535.0, 2147483647.0, -2147483648.0],
        Int32Array.of(0.1, 1.5, 1.9, -1.0, 65535.0, -65535.0, 2147483647.0, -2147483648.0));
    checkNumberArray([0.0, 1.0, 1.0, 4294967295.0, 65535.0, 4294901761.0, 2147483647.0, 2147483648.0],
        Uint32Array.of(0.1, 1.5, 1.9, 4294967295.0, 65535.0, 4294901761.0, 2147483647.0, 2147483648.0));

    checkNumberArray([-1.0, -32766.0, -32767.0, -32768.0, -32769.0, -65535.0, -65536.0, -65537.0, 0.0, 12.0, 255.0,
            32766.0, 32765.0, 32768.0, 32769.0, 65534.0, 65535.0, 65536.0, 100000.0, 2147483647.0, -2147483648.0],
        Int32Array.of(-1.001, -32766.999, -32767.0, -32768.0, -32769.0, -65535.0, -65536.0, -65537.0, 0.0, 12.0, 255.0,
            32766.0, 32765.0, 32768.0, 32769.0, 65534.0, 65535.0, 65536.0, 100000.0, 2147483647.0, -2147483648.0));
    checkNumberArray([4294967295.0, 4294934530.0, 4294934529.0, 4294934528.0, 4294934527.0, 4294901761.0, 4294901760.0,
            4294901759.0, 0.0, 12.0, 255.0, 32766.0, 32765.0, 32768.0, 32769.0, 65534.0, 65535.0, 65536.0, 100000.0,
            2147483647.0, 2147483648.0],
        Uint32Array.of(-1.0, -32766.0, -32767.0, -32768.0, -32769.0, -65535.0, -65536.0, -65537.0, 0.0, 12.0, 255.0,
            32766.0, 32765.0, 32768.0, 32769.0, 65534.0, 65535.0, 65536.0, 100000.0, 2147483647.0, -2147483648.0));

    checkNumberArray([4294967295.0, 4294934530.0, 4294934529.0, 4294934528.0, 4294934527.0, 4294901761.0,
            4294901760.0, 4294901759.0, 0.0, 12.0, 255.0, 32766.0, 32765.0, 32768.0, 32769.0, 65534.0, 65535.0,
            65536.0, 100000.0, 2147483647.0, 2147483648.0],
        Uint32Array.of(4294967295.1, 4294934530.5, 4294934529.9, 4294934528.0, 4294934527.0, 4294901761.0,
            4294901760.0, 4294901759.0, 0.0, 12.0, 255.0, 32766.0, 32765.0, 32768.0, 32769.0, 65534.0, 65535.0,
            65536.0, 100000.0, 2147483647.0, 2147483648.0));
}

function testF32OfNumber() {
    checkNumberArray(new ValueArray<double>(0), Float32Array.of());

    let arr = prepareTestData(127, 1.0);
    checkNumberArray(arr, Float32Array.of(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
        16.0, 17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0, 33.0, 34.0,
        35.0, 36.0, 37.0, 38.0, 39.0, 40.0, 41.0, 42.0, 43.0, 44.0, 45.0, 46.0, 47.0, 48.0, 49.0, 50.0, 51.0, 52.0, 53.0,
        54.0, 55.0, 56.0, 57.0, 58.0, 59.0, 60.0, 61.0, 62.0, 63.0, 64.0, 65.0, 66.0, 67.0, 68.0, 69.0, 70.0, 71.0, 72.0,
        73.0, 74.0, 75.0, 76.0, 77.0, 78.0, 79.0, 80.0, 81.0, 82.0, 83.0, 84.0, 85.0, 86.0, 87.0, 88.0, 89.0, 90.0, 91.0,
        92.0, 93.0, 94.0, 95.0, 96.0, 97.0, 98.0, 99.0, 100.0, 101.0, 102.0, 103.0, 104.0, 105.0, 106.0, 107.0, 108.0,
        109.0, 110.0, 111.0, 112.0, 113.0, 114.0, 115.0, 116.0, 117.0, 118.0, 119.0, 120.0, 121.0, 122.0, 123.0, 124.0,
        125.0, 126.0, 127.0));

    checkNumberArray([1, 65535, -65535, NaN, Infinity], Float32Array.of(1, 65535, -65535, NaN, Infinity));

    checkNumberArray([1, NaN, Infinity, 0.00001f, Infinity, -Infinity, -Infinity, -0.6e+16f, Infinity, -3.31899e+38f,
            3.31899e+38f, -3.40282346e+38f, 3.40282346e+38f, -3.40282346e+38f, 3.40282346e+38f, -3.40282346e+38f,
            3.40282346e+38f, -3.40282346e+38f, 3.40282346e+38f, -Infinity, Infinity],
        Float32Array.of(1.0, NaN, Infinity, 0.00001, 1.7e+308, -Infinity, -1.7e+308, -0.6e+16, 0.4e+105,
            -3.31899e+38, 3.31899e+38, -3.4028234663852886e+38, 3.4028234663852886e+38, -3.402823466385289e+38,
            3.402823466385289e+38, -3.4028235677973359e+38, 3.4028235677973359e+38, -3.4028235677973362e+38,
            3.4028235677973362e+38, -3.4028235677973362e+38 - 1.0e+23, 3.4028235677973362e+38 + 1.0e+23));
}

function testF64OfNumber() {
    checkNumberArray(new ValueArray<double>(0), Float64Array.of());

    let arr = prepareTestData(127, 1.0);
    checkNumberArray(arr, Float64Array.of(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
        16.0, 17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0, 33.0, 34.0,
        35.0, 36.0, 37.0, 38.0, 39.0, 40.0, 41.0, 42.0, 43.0, 44.0, 45.0, 46.0, 47.0, 48.0, 49.0, 50.0, 51.0, 52.0, 53.0,
        54.0, 55.0, 56.0, 57.0, 58.0, 59.0, 60.0, 61.0, 62.0, 63.0, 64.0, 65.0, 66.0, 67.0, 68.0, 69.0, 70.0, 71.0, 72.0,
        73.0, 74.0, 75.0, 76.0, 77.0, 78.0, 79.0, 80.0, 81.0, 82.0, 83.0, 84.0, 85.0, 86.0, 87.0, 88.0, 89.0, 90.0, 91.0,
        92.0, 93.0, 94.0, 95.0, 96.0, 97.0, 98.0, 99.0, 100.0, 101.0, 102.0, 103.0, 104.0, 105.0, 106.0, 107.0, 108.0,
        109.0, 110.0, 111.0, 112.0, 113.0, 114.0, 115.0, 116.0, 117.0, 118.0, 119.0, 120.0, 121.0, 122.0, 123.0, 124.0,
        125.0, 126.0, 127.0));

    checkNumberArray([1, 65535, -65535, NaN, Infinity], Float64Array.of(1, 65535, -65535, NaN, Infinity));
    checkNumberArray([1, NaN, Infinity, 0.00001, 1.7e+308, -Infinity, -1.7e+308, -0.6e+16, 0.4e+105],
        Float64Array.of(1, NaN, Infinity, 0.00001, 1.7e+308, -Infinity, -1.7e+308, -0.6e+16, 0.4e+105));
}

function testIU64OfBigInt() {
    checkBigIntArray(new FixedArray<bigint>(0), BigInt64Array.of());

    const length: int = 127;
    let arr: FixedArray<bigint> = new FixedArray<bigint>(length);
    for (let i = 0; i < length; i++) {
        const v = new BigInt(i + 1);
        arr[i] = v;
    }
    checkBigIntArray(arr, BigInt64Array.of(1n, 2n, 3n, 4n, 5n, 6n, 7n, 8n, 9n, 10n, 11n, 12n, 13n, 14n, 15n, 16n, 17n,
        18n, 19n, 20n, 21n, 22n, 23n, 24n, 25n, 26n, 27n, 28n, 29n, 30n, 31n, 32n, 33n, 34n, 35n, 36n, 37n, 38n, 39n,
        40n, 41n, 42n, 43n, 44n, 45n, 46n, 47n, 48n, 49n, 50n, 51n, 52n, 53n, 54n, 55n, 56n, 57n, 58n, 59n, 60n, 61n,
        62n, 63n, 64n, 65n, 66n, 67n, 68n, 69n, 70n, 71n, 72n, 73n, 74n, 75n, 76n, 77n, 78n, 79n, 80n, 81n, 82n, 83n,
        84n, 85n, 86n, 87n, 88n, 89n, 90n, 91n, 92n, 93n, 94n, 95n, 96n, 97n, 98n, 99n, 100n, 101n, 102n, 103n, 104n,
        105n, 106n, 107n, 108n, 109n, 110n, 111n, 112n, 113n, 114n, 115n, 116n, 117n, 118n, 119n, 120n, 121n, 122n, 123n,
        124n, 125n, 126n, 127n));
    checkBigIntArray(arr, BigUint64Array.of(1n, 2n, 3n, 4n, 5n, 6n, 7n, 8n, 9n, 10n, 11n, 12n, 13n, 14n, 15n, 16n, 17n,
        18n, 19n, 20n, 21n, 22n, 23n, 24n, 25n, 26n, 27n, 28n, 29n, 30n, 31n, 32n, 33n, 34n, 35n, 36n, 37n, 38n, 39n,
        40n, 41n, 42n, 43n, 44n, 45n, 46n, 47n, 48n, 49n, 50n, 51n, 52n, 53n, 54n, 55n, 56n, 57n, 58n, 59n, 60n, 61n,
        62n, 63n, 64n, 65n, 66n, 67n, 68n, 69n, 70n, 71n, 72n, 73n, 74n, 75n, 76n, 77n, 78n, 79n, 80n, 81n, 82n, 83n,
        84n, 85n, 86n, 87n, 88n, 89n, 90n, 91n, 92n, 93n, 94n, 95n, 96n, 97n, 98n, 99n, 100n, 101n, 102n, 103n, 104n,
        105n, 106n, 107n, 108n, 109n, 110n, 111n, 112n, 113n, 114n, 115n, 116n, 117n, 118n, 119n, 120n, 121n, 122n, 123n,
        124n, 125n, 126n, 127n));

    checkBigIntArray([1n, BigInt(1.e+5), 0n, 0n, BigInt(-0.6e+16), 0n, -18014398509481983n, 18014398509481983n,
            -45035996273704959n],
        BigInt64Array.of(1n, BigInt(1.e+5), 0n, 0n, BigInt(-0.6e+16), 0n, -18014398509481983n, 18014398509481983n,
            -45035996273704959n));
    checkBigIntArray([1n, BigInt(1.e+5), 0n, 0n, 18440744073709551616n, 0n, 18428729675200069633n, 18014398509481983n,
            18401708077435846657n],
        BigUint64Array.of(1n, BigInt(1.e+5), 0n, 0n, BigInt(-0.6e+16), 0n, -18014398509481983n, 18014398509481983n,
            -45035996273704959n));
    checkBigIntArray([1n, 100000n, 0n, 0n, 18440744073709551616n, 0n, 18428729675200069633n, 18014398509481983n,
            18401708077435846657n],
        BigUint64Array.of(1n, 100000n, 0n, 0n, 18440744073709551616n, 0n, 18428729675200069633n, 18014398509481983n,
            18401708077435846657n));
}

function prepareTestData(length: int, from: int): ValueArray<int> {
    let fixarr: ValueArray<int> = new ValueArray<int>(length);
    let v: int = from;
    for (let i = 0; i < length; i++) {
        fixarr[i] = v++;
    }
    return fixarr;
}

function prepareTestData(length: int, from: number): ValueArray<double> {
    let fixarr: ValueArray<double> = new ValueArray<double>(length);
    let v: number = from;
    for (let i = 0; i < length; i++) {
        fixarr[i] = v++;
    }
    return fixarr;
}

function prepareTestData16(length: int, from: int): ValueArray<int> {
    let fixarr: ValueArray<int> = new ValueArray<int>(length);
    let v: int = from;
    for (let i = 0; i < length; i++) {
        fixarr[i] = ((v << 16) >> 16);
        v++;
    }
    return fixarr;
}

function prepareTestData32(length: int, from: int): ValueArray<int> {
    let fixarr: ValueArray<int> = new ValueArray<int>(length);
    let v: int = from;
    for (let i = 0; i < length; i++) {
        fixarr[i] = (v << 0) >> 0;
        v++;
    }
    return fixarr;
}

function makeInt16Array(length: int, from: int): Int16Array {
    let arr = new Int16Array(length);
    let v: int = from;
    for (let i = 0; i < length; i++) {
        arr[i] = ((v << 16) >> 16);
        v++;
    }
    return arr;
}

function makeUint16Array(length: int, from: int): Uint16Array {
    let arr = new Uint16Array(length);
    let v: int = from;
    for (let i = 0; i < length; i++) {
        arr[i] = ((v << 16) >>> 16);
        v++;
    }
    return arr;
}

function makeInt32Array(length: int, from: int): Int32Array {
    let arr = new Int32Array(length);
    let v: int = from;
    for (let i = 0; i < length; i++) {
        arr[i] = v;
        v++;
    }
    return arr;
}

function makeUint32Array(length: int, from: int): Uint32Array {
    let arr = new Uint32Array(length);
    let v: int = from;
    for (let i = 0; i < length; i++) {
        arr[i] = v;
        v++;
    }
    return arr;
}

function checkNumberArray(expected: ValueArray<double>, data: ArrayLike<number>): void {
    arktest.assertEQ(data.length, expected.length, 'Unexpected array length');
    for (let i: int = 0; i < expected.length; i++) {
        arktest.assertTrue((isNaN(data[i]) && isNaN(expected[i])) || (data[i] == expected[i]),
            'Unexpected element with index ' + i + ': expected ' + expected[i] + ' but was ' + data[i]);
    }
}

function checkIntArray(expected: ValueArray<int>, data: ArrayLike<number>): void {
    arktest.assertEQ(data.length, expected.length, 'Unexpected array length');
    for (let i: int = 0; i < expected.length; i++) {
        arktest.assertTrue(data[i] == expected[i],
            'Unexpected element with index ' + i + ': expected ' + expected[i] + ' but was ' + data[i]);
    }
}

function checkBigIntArray(expected: FixedArray<BigInt>, data: ArrayLike<BigInt>): void {
    arktest.assertEQ(data.length, expected.length, 'Unexpected array length');
    for (let i: int = 0; i < expected.length; i++) {
        arktest.assertEQ(data[i], expected[i], 'Unexpected element with index ' + i);
    }
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('typedArrays.of');
    testSuite.addTest('BigInt64,BigUint64 Arrays: of<BigInt> test', testIU64OfBigInt);
    testSuite.addTest('Int8,Uint8 Arrays: of<int> test', testIU8OfInt);
    testSuite.addTest('Int16,Uint16 Arrays: of<int> test', testIU16OfInt);
    testSuite.addTest('Int32,Uint32 Arrays: of<int> test', testIU32OfInt);
    testSuite.addTest('Int32,Uint32 Arrays: of<number> test', testIU32OfNumber);
    testSuite.addTest('Float32 Arrays: of<number> test', testF32OfNumber);
    testSuite.addTest('Float64 Arrays: of<number> test', testF64OfNumber);
    return testSuite.run();
}
