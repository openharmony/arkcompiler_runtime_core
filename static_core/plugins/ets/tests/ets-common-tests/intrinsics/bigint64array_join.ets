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

const SINGLE = [1];
const ARRAY: Array<Double> = [Long.MIN_VALUE, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, Long.MAX_VALUE];
const EMPTY_JOINED = '-922337203685477580801234567899223372036854775807';
const COMMA_JOINED = '-9223372036854775808,0,1,2,3,4,5,6,7,8,9,9223372036854775807';
const NSIGN_JOINED = '-9223372036854775808###0###1###2###3###4###5###6###7###8###9###9223372036854775807';
const UTF16_JOINED =
    '-9223372036854775808 ΔΘΩ 0 ΔΘΩ 1 ΔΘΩ 2 ΔΘΩ 3 ΔΘΩ 4 ΔΘΩ 5 ΔΘΩ 6 ΔΘΩ 7 ΔΘΩ 8 ΔΘΩ 9 ΔΘΩ 9223372036854775807';
const UTF16_JOINED_1 = '-9223372036854775808Ω0Ω1Ω2Ω3Ω4Ω5Ω6Ω7Ω8Ω9Ω9223372036854775807';

function joinEmpty(): void {
    let a: BigInt64Array = new BigInt64Array(0);
    let s = a.join();
    assertTrue(s.isEmpty());
}

function joinSingle(): void {
    let a: BigInt64Array = new BigInt64Array(SINGLE);
    arktest.assertEQ(a.join(), '1');
    arktest.assertEQ(a.join(','), '1');
}

function joinDefaultDelim(): void {
    let a: BigInt64Array = new BigInt64Array(ARRAY);
    assertEQ(a.join(), COMMA_JOINED);
}

function joinEmptyDelim(): void {
    let a: BigInt64Array = new BigInt64Array(ARRAY);
    arktest.assertEQ(a.join(''), EMPTY_JOINED);
}

function joinCommaDelim(): void {
    let a: BigInt64Array = new BigInt64Array(ARRAY);
    arktest.assertEQ(a.join(','), COMMA_JOINED);
}

function joinNsignDelim(): void {
    let a: BigInt64Array = new BigInt64Array(ARRAY);
    arktest.assertEQ(a.join('###'), NSIGN_JOINED);
}

function joinUtf16Delim(): void {
    let a: BigInt64Array = new BigInt64Array(ARRAY);
    arktest.assertEQ(a.join(' ΔΘΩ '), UTF16_JOINED);
}

function joinUtf16Delim1(): void {
    let a: BigInt64Array = new BigInt64Array(ARRAY);
    arktest.assertEQ(a.join('Ω'), UTF16_JOINED_1);
}

function main(): int {
    const suite = new arktest.ArkTestsuite('BigInt64Array.join() tests')
    suite.addTest('join an empty BigInt64Array', joinEmpty)
    suite.addTest('join a single-element BigInt64Array', joinSingle)
    suite.addTest('join a normal BigInt64Array using default delimiter', joinDefaultDelim)
    suite.addTest('join a normal BigInt64Array using empty delimiter', joinEmptyDelim)
    suite.addTest('join a normal BigInt64Array using comma delimiter', joinCommaDelim)
    suite.addTest('join a normal BigInt64Array using nsign delimiter', joinNsignDelim)
    suite.addTest('join a normal BigInt64Array using utf16 delimiter', joinUtf16Delim)
    suite.addTest('join a normal BigInt64Array using a single utf16 char delimiter', joinUtf16Delim1)
    return suite.run()
}
