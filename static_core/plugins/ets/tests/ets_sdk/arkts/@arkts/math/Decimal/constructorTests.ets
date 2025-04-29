/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

import { Decimal } from "@arkts.math.Decimal";

function t(digits: number[], exponent: number, sign: number, n: number | string) {
    let v = new Decimal(n);
    assertEQ(digits.length, v.d!.length,
        `Decimal(${n}).d.length, actual:${v.d!.length} is not equal to ${digits.length}`);
    for (let i = 0; i < digits.length; i++) {
        assertEQ(digits[i], v.d![i], `Decimal(${n}).d, actual:${v.d!} is not equal to ${digits}`);
    }
    assertEQ(exponent, v.e, `Decimal(${n}).e, actual:${v.e} is not equal to ${exponent}`);
    assertEQ(sign, v.s, `Decimal(${n}).s, actual:${v.s} is not equal to ${sign}`);
}

function t2(expected: string, n: number | string) {
    let v = new Decimal(n).valueOf();
    assertEQ(expected, v, `Decimal(${n}).valueOf(), actual:${v} is not equal to ${expected}`);
}

function config() {
    Decimal.precision = 40;
    Decimal.rounding = 4;
    Decimal.toExpNeg = -9e15;
    Decimal.toExpPos = 9e15;
    Decimal.maxE = 9e15;
    Decimal.minE = -9e15;
}

function testCreateDecimalWithNumber() {
    t([0], 0, 1, 0);
    t([1], 0, -1, -1);
    t([10], 1, -1, -10);

    t([1], 0, 1, 1);
    t([10], 1, 1, 10);
    t([100], 2, 1, 100);
    t([1000], 3, 1, 1000);
    t([10000], 4, 1, 10000);
    t([100000], 5, 1, 100000);
    t([1000000], 6, 1, 1000000);

    t([1], 7, 1, 10000000);
    t([10], 8, 1, 100000000);
    t([100], 9, 1, 1000000000);
    t([1000], 10, 1, 10000000000);
    t([10000], 11, 1, 100000000000);
    t([100000], 12, 1, 1000000000000);
    t([1000000], 13, 1, 10000000000000);

    t([1], 14, -1, -100000000000000);
    t([10], 15, -1, -1000000000000000);

    t([1000000], -1, 1, 1e-1);
    t([100000], -2, -1, -1e-2);
    t([10000], -3, 1, 1e-3);
    t([1000], -4, -1, -1e-4);
    t([100], -5, 1, 1e-5);
    t([10], -6, -1, -1e-6);
    t([1], -7, 1, 1e-7);

    t([1000000], -8, 1, 1e-8);
    t([100000], -9, -1, -1e-9);
    t([10000], -10, 1, 1e-10);
    t([1000], -11, -1, -1e-11);
    t([100], -12, 1, 1e-12);
    t([10], -13, -1, -1e-13);
    t([1], -14, 1, 1e-14);

    t([1000000], -15, 1, 1e-15);
    t([100000], -16, -1, -1e-16);
    t([10000], -17, 1, 1e-17);
    t([1000], -18, -1, -1e-18);
    t([100], -19, 1, 1e-19);
    t([10], -20, -1, -1e-20);
    t([1], -21, 1, 1e-21);
}

function testCreateDecimalWithString() {
    t([9], 0, 1, '9');
    t([99], 1, -1, '-99');
    t([999], 2, 1, '999');
    t([9999], 3, -1, '-9999');
    t([99999], 4, 1, '99999');
    t([999999], 5, -1, '-999999');
    t([9999999], 6, 1, '9999999');

    t([9, 9999999], 7, -1, '-99999999');
    t([99, 9999999], 8, 1, '999999999');
    t([999, 9999999], 9, -1, '-9999999999');
    t([9999, 9999999], 10, 1, '99999999999');
    t([99999, 9999999], 11, -1, '-999999999999');
    t([999999, 9999999], 12, 1, '9999999999999');
    t([9999999, 9999999], 13, -1, '-99999999999999');

    t([9, 9999999, 9999999], 14, 1, '999999999999999');
    t([99, 9999999, 9999999], 15, -1, '-9999999999999999');
    t([999, 9999999, 9999999], 16, 1, '99999999999999999');
    t([9999, 9999999, 9999999], 17, -1, '-999999999999999999');
    t([99999, 9999999, 9999999], 18, 1, '9999999999999999999');
    t([999999, 9999999, 9999999], 19, -1, '-99999999999999999999');
    t([9999999, 9999999, 9999999], 20, 1, '999999999999999999999');
}

function testCreateDecimalWithBinary() {
    t2('0', '0b0');
    t2('0', '0B0');
    t2('-5', '-0b101');
    t2('5', '+0b101');
    t2('1.5', '0b1.1');
    t2('-1.5', '-0b1.1');

    t2('18181', '0b100011100000101.00');
    t2('-12.5', '-0b1100.10');
    t2('343872.5', '0b1010011111101000000.10');
    t2('-328.28125', '-0b101001000.010010');
    t2('-341919.144535064697265625', '-0b1010011011110011111.0010010100000000010');
    t2('97.10482025146484375', '0b1100001.000110101101010110000');
    t2('-120914.40625', '-0b11101100001010010.01101');
    t2('8080777260861123367657', '0b1101101100000111101001111111010001111010111011001010100101001001011101001');
}

function testCreateDecimalWithOctal() {
    t2('8', '0o10');
    t2('-8.5', '-0O010.4');
    t2('8.5', '+0O010.4');
    t2('-262144.000000059604644775390625', '-0o1000000.00000001');
    t2('572315667420.390625', '0o10250053005734.31');
}
function testCreateDecimalWithHex() {
    t2('1', '0x00001');
    t2('255', '0xff');
    t2('-15.5', '-0Xf.8');
    t2('15.5', '+0Xf.8');
    t2('-16777216.00000000023283064365386962890625', '-0x1000000.00000001');
    t2('325927753012307620476767402981591827744994693483231017778102969592507',
        '0xc16de7aa5bf90c3755ef4dea45e982b351b6e00cd25a82dcfe0646abb');
}

function testCreateDecimalWithNaN() {
    t2('NaN', NaN);
    t2('NaN', -NaN);
    t2('NaN', 'NaN');
    t2('NaN', '-NaN');
    t2('NaN', '+NaN');
}

function testCreateDecimalWithInfinity() {
    t2('Infinity', Infinity);
    t2('-Infinity', -Infinity);
    t2('Infinity', 'Infinity');
    t2('-Infinity', '-Infinity');
    t2('Infinity', '+Infinity');
}

function main() {
    const suite = new ArkTestsuite("Decimal Constructor API tests");
    suite.addTest("Decimal constructor with number", testCreateDecimalWithNumber);
    suite.addTest("Decimal constructor with string", testCreateDecimalWithString);
    suite.addTest("Decimal constructor with binary", testCreateDecimalWithBinary);
    suite.addTest("Decimal constructor with octal", testCreateDecimalWithOctal);
    suite.addTest("Decimal constructor with hex", testCreateDecimalWithHex);
    suite.addTest("Decimal constructor with NaN", testCreateDecimalWithNaN);
    suite.addTest("Decimal constructor with Infinity", testCreateDecimalWithInfinity);
    config();
    return suite.run();
}