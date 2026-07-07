/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

import { PI } from 'std/math/consts';

const nan = Double.NaN;
const posInf = Double.POSITIVE_INFINITY;
const negInf = Double.NEGATIVE_INFINITY;

function test(result: boolean, name: String): int {
    if (result) {
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.println('PASSED: ' + name);
        return 0;
    }
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.println('FAILED: ' + name);
    return 1;
}

function testAbs(): boolean {
    return Double.isNaN(abs(nan)) &&
        Double.compare(abs(Double.MAX_VALUE), Double.MAX_VALUE) &&
        Double.compare(abs(-Double.MAX_VALUE), Double.MAX_VALUE) &&
        !Double.isFinite(abs(posInf)) &&
        !Double.isFinite(abs(negInf));
}

function testSin(): boolean {
    return Double.compare(sin(PI), 0) &&
        Double.compare(sin(PI / 6), 0.5) &&
        Double.isNaN(sin(nan)) &&
        Double.isNaN(sin(posInf)) &&
        Double.isNaN(sin(negInf));
}

function testCos(): boolean {
    return Double.compare(cos(2 * PI), 1) &&
        Double.compare(cos(PI / 3), 0.5) &&
        Double.isNaN(cos(nan)) &&
        Double.isNaN(cos(posInf)) &&
        Double.isNaN(cos(negInf));
}

function testAcosh(): boolean {
    return Double.isNaN(acosh(nan)) &&
        !Double.isFinite(acosh(posInf)) &&
        !Double.isFinite(acosh(negInf));
}

function testAcos(): boolean {
    return Double.isNaN(acos(nan)) &&
        Double.isNaN(acos(posInf)) &&
        Double.isNaN(acos(negInf));
}

function testAsin(): boolean {
    return Double.isNaN(asin(nan)) &&
        Double.isNaN(asin(posInf)) &&
        Double.isNaN(asin(negInf));
}

function testAtan(): boolean {
    return Double.isNaN(atan(nan)) &&
        Double.compare(atan(posInf), PI / 2) &&
        Double.compare(atan(negInf), -PI / 2) &&
        (atan(0) == 0) &&
        Float.compare(atan(1), (PI / 4).toFloat());
}

function testSignbit(): boolean {
    return signbit(nan) &&
        !signbit(posInf) &&
        signbit(negInf);
}

function testRound(): boolean {
    return Double.isNaN(round(nan)) &&
        !Double.isFinite(round(posInf)) &&
        !Double.isFinite(round(negInf));
}

function testFloor(): boolean {
    return Double.isNaN(floor(nan)) &&
        !Double.isFinite(floor(negInf)) &&
        !Double.isFinite(floor(posInf));
}

function testCeil(): boolean {
    return Double.isNaN(ceil(nan)) &&
        !Double.isFinite(ceil(negInf)) &&
        !Double.isFinite(ceil(posInf));
}

function testPower(): boolean {
    return Double.isNaN(power(nan, posInf)) &&
        !Double.isFinite(power(posInf, posInf));
}

function testMod(): boolean {
    return Double.isNaN(mod(nan, posInf)) &&
        !Double.isFinite(mod(posInf, posInf));
}

function testLog(): boolean {
    return Double.isNaN(log(nan)) &&
        Double.isNaN(log(negInf)) &&
        !Double.isFinite(log(posInf));
}

function testLog2(): boolean {
    return Double.isNaN(log2(nan)) &&
        Double.isNaN(log2(negInf)) &&
        !Double.isFinite(log2(posInf));
}

function testLog10(): boolean {
    return Double.isNaN(log10(nan)) &&
        Double.isNaN(log10(negInf)) &&
        !Double.isFinite(log10(posInf));
}

function testExp(): boolean {
    return Double.isNaN(exp(nan)) &&
        !Double.isFinite(exp(posInf)) &&
        (exp(negInf) == 0);
}

function testClz32(): boolean {
    return (clz32(-0x1) == 0) &&
        (clz32(0x0000FFFF) == 16) &&
        (clz32(0x0) == 32);
}

function testClz64(): boolean {
    return (clz64(-0x1) == 0) &&
        (clz64(0x0000FFFFFFFFFFFF) == 16) &&
        (clz64(0x0) == 64);
}

function main(): int {
    let failures: int = 0;
    failures += test(testAbs(), 'Test abs()');
    failures += test(testSin(), 'Test sin()');
    failures += test(testCos(), 'Test cos()');
    failures += test(testAcosh(), 'Test acosh()');
    failures += test(testAcos(), 'Test acos()');
    failures += test(testAsin(), 'Test asin()');
    failures += test(testAtan(), 'Test atan()');
    // TODO: curently fails on arm64
    // TODO failures += test(testSignbit(), 'Test signbit()');
    failures += test(testRound(), 'Test round()');
    failures += test(testCeil(), 'Test ceil()');
    failures += test(testFloor(), 'Test floor()');
    failures += test(testPower(), 'Test power()');
    failures += test(testMod(), 'Test mod()');
    failures += test(testLog(), 'Test log()');
    failures += test(testLog2(), 'Test log2()');
    failures += test(testLog10(), 'Test log10()');
    failures += test(testClz32(), 'Test clz32()');
    failures += test(testClz64(), 'Test clz64()');
    failures += test(testExp(), 'Test exp()');
    if (failures == 0) {
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.println('PASSED: All tests run');
    } else {
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.println('FAILED: All tests run');
    }
    return failures;
}
