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

function check(currentAnswer: number, correctAnswer: number, test: string) {
    if (currentAnswer != correctAnswer) {
        if (!Number.isNaN(currentAnswer) || !Number.isNaN(correctAnswer)) {
            arktest.assertTrue( false, 'incorrect ' + test + ' answer ' + currentAnswer + ' instead of ' + correctAnswer)
            return 1;
        }
    }
    return 0;
}

function main() {
    let fails = 0;
    let tests: string[] = [
        // empty and whitespace
        '',
        '\n',
        '\t',
        '    ',
        '   123   ',
        // basic integers
        '123',
        '-1',
        '00015',
        // sign + zero
        '-0',
        '+0',
        // sign + integer
        '+5',
        ' +5',
        ' +5a',
        // decimals
        '9.8',
        '.5',
        '-1.11111',
        // Infinity
        'Infinity',
        '+Infinity',
        '-Infinity',
        // scientific notation
        '1e8',
        '+1234.5678E-90',
        '10e10000',
        // invalid strings
        'NaN',
        'nan',
        'aaa',
        'e8',
        '1e',
        '1rew',
        // hex literals
        '0x11',
        ' 0x1',
        '0x01R',
        '0xR',
        '0x00 001',
        '0x11 abacaba',
        '+0x1',
        // octal literals
        '0o123',
        '+0o10',
        '0o',
        // binary literals
        '0b0101010',
        '0b222',
        '+0b1',
        '0b',
        '0b1.01',
        '0b1e10',
        // mixed edge cases
        '00b0'
    ]
    const correctCts: number[] = [
        // empty and whitespace
        0,
        0,
        0,
        0,
        123,
        // basic integers
        123,
        -1,
        15,
        // sign + zero
        0,
        0,
        // sign + integer
        5,
        5,
        NaN,
        // decimals
        9.8,
        0.5,
        -1.11111,
        // Infinity
        Infinity,
        Infinity,
        -Infinity,
        // scientific notation
        100000000,
        1234.5678E-90,
        Infinity,
        // invalid strings
        NaN,
        NaN,
        NaN,
        NaN,
        NaN,
        NaN,
        // hex literals
        17,
        1,
        NaN,
        NaN,
        NaN,
        NaN,
        NaN,
        // octal literals
        83,
        NaN,
        NaN,
        // binary literals
        42,
        NaN,
        NaN,
        NaN,
        NaN,
        NaN,
        // mixed edge cases
        NaN
    ];
    const correctPfs: number[] = [
        // empty and whitespace
        NaN,
        NaN,
        NaN,
        NaN,
        123,
        // basic integers
        123,
        -1,
        15,
        // sign + zero
        0,
        0,
        // sign + integer
        5,
        5,
        5,
        // decimals
        9.8,
        0.5,
        -1.11111,
        // Infinity
        Infinity,
        Infinity,
        -Infinity,
        // scientific notation
        100000000,
        1234.5678E-90,
        Infinity,
        // invalid strings
        NaN,
        NaN,
        NaN,
        NaN,
        1,
        1,
        // hex literals
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        // octal literals
        0,
        0,
        0,
        // binary literals
        0,
        0,
        0,
        0,
        0,
        0,
        // mixed edge cases
        0
    ];

    const forceLog = false;
    for (let testId = 0; testId < tests.length; testId++) {
        let test = tests[testId];
        if (forceLog) {
            // CC-OFFNXT(G.AOD.03) current test suite code style
            console.log('[' + test + ']');
        }
        let ctorAnswer = (Number(test)); // change to .valueOf to run script in TS
        let parseAnswer = Number.parseFloat(test);
        fails += check(ctorAnswer, correctCts[testId], 'Number(' + test + ')');
        fails += check(parseAnswer, correctPfs[testId], 'parseFloat(' + test + ')');
    }
    return fails;
}