/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

function emptyStringTest() {
    arktest.assertEQ(string.fromCharCode(), '', 'Must be an empty string');
    arktest.assertEQ(string.fromCharCode().length, 0, 'String length of an empty string must be equal to 0');
}

function compressibleStringTest() {
    arktest.assertEQ(string.fromCharCode(0x43 as number, 65.992 as number), 'CA', "Must be 'CA'");
    arktest.assertEQ(string.fromCharCode(0x43 as number, 65.992 as number).length, 2, "String length of 'CA' must be equal to 2");
    arktest.assertEQ(string.fromCharCode(0xA as number, 0xA as number), '\n\n', "Must be '\\n\\n'");
    arktest.assertEQ(string.fromCharCode(0xA as number, 0xA as number).length, 2, "String length of '\\n\\n' must be equal to 2");
    arktest.assertEQ(string.fromCharCode(0xA as number, 0x9 as number, 0xA as number), '\n\t\n', "Must be '\\n\\t\\n'");
    arktest.assertEQ(string.fromCharCode(0xA as number, 0x9 as number, 0xA as number).length, 3, "String length of '\\n\\t\\n' must be equal to 3");
    arktest.assertEQ(string.fromCharCode(0xA as number, 0xD as number), '\n\r', "Must be '\\n\\r'");
    arktest.assertEQ(string.fromCharCode(0xA as number, 0xD as number).length, 2, "String length of '\\n\\r' must be equal to 2");
    arktest.assertEQ(string.fromCharCode(0x41, 0x42, 0x43, 0x44), 'ABCD', "Must be 'ABCD'");
    arktest.assertEQ(string.fromCharCode(0x41, 0x42, 0x43, 0x44).length, 4, "String length of 'ABCD' must be equal to 4");
}

function compressibleSingleStringTest() {
    arktest.assertEQ(string.fromCharCode(65.992 as number), 'A', "Must be 'A'");
    arktest.assertEQ(string.fromCharCode(65.992 as number).length, 1, "String length of 'A' must be equal to 1");
    arktest.assertEQ(string.fromCharCode(0x41), 'A', "Must be 'A'");
    arktest.assertEQ(string.fromCharCode(0x41).length, 1, "String length of 'A' must be equal to 1");
    arktest.assertEQ(string.fromCharCode(0x43 as number), 'C', "Must be 'C'");
    arktest.assertEQ(string.fromCharCode(0x43 as number).length, 1, "String length of 'C' must be equal to 1");
    arktest.assertEQ(string.fromCharCode(0x9 as number), '\t', "Must be '\\t'");
    arktest.assertEQ(string.fromCharCode(0x9 as number).length, 1, "String length of '\\t' must be equal to 1");
    arktest.assertEQ(string.fromCharCode(0xA as number), '\n', "Must be '\\n'");
    arktest.assertEQ(string.fromCharCode(0xA as number).length, 1, "String length of '\\n' must be equal to 1");
    arktest.assertEQ(string.fromCharCode(0xA as number, 0xD as number), '\n\r', "Must be '\\n\\r'");
    arktest.assertEQ(string.fromCharCode(0xA as number, 0xD as number).length, 2, "String length of '\\n\\r' must be equal to 2");
}

function incompressibleStringTest() {
    arktest.assertEQ(string.fromCharCode(0x3B1, 0x41, 0x42, 0x43, 0x44), 'αABCD', "Must be 'αABCD'");
    arktest.assertEQ(string.fromCharCode(0x3B1, 0x41, 0x42, 0x43, 0x44).length, 5, "String length of 'αABCD' must be equal to 5");
    arktest.assertEQ(string.fromCharCode(0x41, 0x3B1, 0x42, 0x43, 0x44), 'AαBCD', "Must be 'AαBCD'");
    arktest.assertEQ(string.fromCharCode(0x41, 0x3B1, 0x42, 0x43, 0x44).length, 5, "String length of 'AαBCD' must be equal to 5");
    arktest.assertEQ(string.fromCharCode(0x41, 0x42, 0x43, 0x44, 0x3B1), 'ABCDα', "Must be 'ABCDα'");
    arktest.assertEQ(string.fromCharCode(0x41, 0x42, 0x43, 0x44, 0x3B1).length, 5, "String length of 'ABCDα' must be equal to 5");
    arktest.assertEQ(string.fromCharCode(0x3B1, 0x3B2, 0x3B3), 'αβγ', "Must be 'αβγ'");
    arktest.assertEQ(string.fromCharCode(0x3B1, 0x3B2, 0x3B3).length, 3, "String length of 'αβγ' must be equal to 3");
    arktest.assertEQ(string.fromCharCode(0x43 as number, 0x3B2 as number, 0xD799 as number,
                                 0x1D798 as number, 65.992 as number),
        'Cβ힙힘A', "Must be 'Cβ힙힘A'");
    arktest.assertEQ(string.fromCharCode(65435 as number, 65435 as number), 'ﾛﾛ', "Error! Must be 'ﾛﾛ'");
    arktest.assertEQ(string.fromCharCode(65436 as number, 65436 as number), 'ﾜﾜ', "Must be 'ﾜﾜ'");
    arktest.assertEQ(string.fromCharCode(65437 as number, 65437 as number), 'ﾝﾝ', "Error! Must be 'ﾝﾝ'");
}

function incompressibleSingleStringTest() {
    arktest.assertEQ(string.fromCharCode(0x3B2 as number), 'β', "Must be 'β'");
    arktest.assertEQ(string.fromCharCode(0x3B2 as number).length, 1, "String length of 'β' must be equal to 1");
    arktest.assertEQ(string.fromCharCode(0x103B2 as number), 'β', "Must be 'β'");
    arktest.assertEQ(string.fromCharCode(0x103B2 as number).length, 1, "String length of 'β' must be equal to 1");
    arktest.assertEQ(string.fromCharCode(0xD799 as number), '힙', "Must be '힙'");
    arktest.assertEQ(string.fromCharCode(0xD799 as number).length, 1, "String length of '힙' must be equal to 1");
    arktest.assertEQ(string.fromCharCode(65435 as number), 'ﾛ', "Error! Must be 'ﾛ'");
    arktest.assertEQ(string.fromCharCode(65436 as number), 'ﾜ', "Must be 'ﾜ'");
    arktest.assertEQ(string.fromCharCode(65437 as number), 'ﾝ', "Error! Must be 'ﾝ'");
}

function falsePositiveWrongSymbolFlagTest() {
    arktest.assertEQ(string.fromCharCode(0x1000003B2 as number, 0x1000003B2 as number), 'ββ', "Must be 'ββ'");
    arktest.assertEQ(string.fromCharCode(0x1000003B2 as number, 0x1000003B2 as number).length, 2, "String length of 'ββ' must be equal to 2");
}

function falsePositiveWrongSymbolFlagSingleTest() {
    arktest.assertEQ(string.fromCharCode(0x1000003B2 as number), 'β', "Must be 'β'");
    arktest.assertEQ(string.fromCharCode(0x1000003B2 as number).length, 1, "String length of 'β' must be equal to 1");
}

function negativeCasesTest() {
    arktest.assertEQ(string.fromCharCode(-100), 'ﾜ', "Must be 'ﾜ'");
    arktest.assertEQ(string.fromCharCode(-100).length, 1);
    arktest.assertEQ(string.fromCharCode(-65535), '\u{1}', 'CharCode[0] of a string built from -65535 must be 1');
    arktest.assertEQ(string.fromCharCode(-65535).length, 1);
    arktest.assertEQ(string.fromCharCode(-65535, -65535), '\u{1}\u{1}', 'CharCode[0] of a string built from -65535 must be 1');
    arktest.assertEQ(string.fromCharCode(-65535, -65535).length, 2);
    arktest.assertEQ(string.fromCharCode(-65535, -100), '\u{1}ﾜ', 'CharCode[0] of a string built from -65535, -100 must be 1');
    arktest.assertEQ(string.fromCharCode(-65535, -100).length, 2);
    arktest.assertEQ(string.fromCharCode(-100, -65535), 'ﾜ\u{1}', 'Char[0] of a string built from -100, -65535 must be ﾜ');
    arktest.assertEQ(string.fromCharCode(-100, -65535).length, 2);
    arktest.assertEQ(string.fromCharCode(-65536, -65536), '\u{0}\u{0}', 'CharCode[0] of a string built from -65536 must be 0');
    arktest.assertEQ(string.fromCharCode(-65536, -65536).length, 2);
    arktest.assertEQ(string.fromCharCode(0xffff0066, -100), 'fﾜ', "Must be 'fﾜ'");
    arktest.assertEQ(string.fromCharCode(0xffff0066, -100).length, 2, "String length of 'fﾜ' must be equal to 2");
    arktest.assertEQ(string.fromCharCode(-100, 0xffff0066), 'ﾜf', "Must be 'ﾜf'");
    arktest.assertEQ(string.fromCharCode(-100, 0xffff0066).length, 2, "String length of 'ﾜf' must be equal to 2");
}

function negativeSingleCasesTest() {
    arktest.assertEQ(string.fromCharCode(-100), 'ﾜ', "Must be 'ﾜ'");
    arktest.assertEQ(string.fromCharCode(-100).length, 1);
    arktest.assertEQ(string.fromCharCode(-65535), '\u{1}', 'CharCode[0] of a string built from -65535 must be 1');
    arktest.assertEQ(string.fromCharCode(-65535).length, 1);
    arktest.assertEQ(string.fromCharCode(-65536), '\u{0}', 'CharCode[0] of a string built from -65536 must be 0');
    arktest.assertEQ(string.fromCharCode(-65536).length, 1);
    arktest.assertEQ(string.fromCharCode(-1), '\u{ffff}', 'CharCode[0] of a string built from -1 must be 65535');
    arktest.assertEQ(string.fromCharCode(-1).length, 1);
    arktest.assertEQ(string.fromCharCode(0xffff0066), 'f', "Must be 'f'");
    arktest.assertEQ(string.fromCharCode(0xffff0066).length, 1, "String length of 'f' must be equal to 1");
}

function saturationCasesTest() {
    arktest.assertEQ(string.fromCharCode(65535, 65535), '\u{ffff}\u{ffff}', 'CharCode[0] of a string built from 65535 must be 65535');
    arktest.assertEQ(string.fromCharCode(65535, 65535).length, 2);
    arktest.assertEQ(string.fromCharCode(65536, 65536), '\u{0}\u{0}', 'CharCode[0] of a string built from 65536 must be 0');
    arktest.assertEQ(string.fromCharCode(65536, 65536).length, 2);
    arktest.assertEQ(string.fromCharCode(4294901862, 4294901862), 'ff', "Must be 'ff'");
    arktest.assertEQ(string.fromCharCode(4294901862, 4294901862).length, 2, "String length of 'ff' must be equal to 2");
}

function saturationSingleCasesTest() {
    arktest.assertEQ(string.fromCharCode(65535), '\u{ffff}', 'CharCode[0] of a string built from 65535 must be 65535');
    arktest.assertEQ(string.fromCharCode(65535).length, 1);
    arktest.assertEQ(string.fromCharCode(65536), '\u{0}', 'CharCode[0] of a string built from 65536 must be 0');
    arktest.assertEQ(string.fromCharCode(65536).length, 1);
    arktest.assertEQ(string.fromCharCode(4294901862), 'f', "Must be 'f'");
    arktest.assertEQ(string.fromCharCode(4294901862).length, 1, "String length of 'f' must be equal to 1");
}

function overflowCasesTest() {
    arktest.assertEQ(string.fromCharCode(NaN, NaN), '\u{0}\u{0}', 'CharCode[0] of a string built from NaN must be 0');
    arktest.assertEQ(string.fromCharCode(NaN, NaN).length, 2);
    arktest.assertEQ(string.fromCharCode(Infinity, Infinity), '\u{0}\u{0}', 'CharCode[0] of a string built from Infinity must be 0');
    arktest.assertEQ(string.fromCharCode(Infinity, Infinity).length, 2);
    arktest.assertEQ(string.fromCharCode(9007199254740992.0 as number, 9007199254740991.0 as number), '\u{0}\u{ffff}',
        'CharCode[0] of a string built from 2^53, 2^53-1 must be 0');
    arktest.assertEQ(string.fromCharCode(9007199254740991.0 as number, 9007199254740992.0 as number), '\u{ffff}\u{0}',
        'CharCode[0] of a string built from 2^53-1, 2^53 must be 65535');
    arktest.assertEQ(string.fromCharCode(-9007199254740992.0 as number, -9007199254740991.0 as number), '\u{0}\u{1}',
        'CharCode[0] of a string built from -2^53, -(2^53-1) must be 0');
    arktest.assertEQ(string.fromCharCode(-9007199254740991.0 as number, -9007199254740992.0 as number), '\u{1}\u{0}',
        'CharCode[0] of a string built from -(2^53-1), -2^53 must be 1');
    arktest.assertEQ(string.fromCharCode(-9007199254740993.0 as number, -9007199254740993.0 as number), '\u{0}\u{0}',
        'CharCode[0] of a string built from -(2^53+1) must be 0');
    arktest.assertEQ(string.fromCharCode(18446744073709551615.0 as number, 18446744073709551615.0 as number), '\u{0}\u{0}',
        'CharCode[0] of a string built from 2^64-1 must be 0');
    arktest.assertEQ(string.fromCharCode(18446744073709551616.0 as number, 18446744073709551616.0 as number), '\u{0}\u{0}',
        'CharCode[0] of a string built from 2^64 must be 0');
    arktest.assertEQ(string.fromCharCode(-18446744073709551616.0 as number, -18446744073709551616.0 as number), '\u{0}\u{0}',
        'CharCode[0] of a string built from -2^64 must be 0');
    arktest.assertEQ(string.fromCharCode(18446744073709551617.0 as number, 18446744073709551617.0 as number), '\u{0}\u{0}',
        'CharCode[0] of a string built from 2^64+1 must be 0');
    arktest.assertEQ(string.fromCharCode(-18446744073709551617.0 as number, -18446744073709551617.0 as number), '\u{0}\u{0}',
        'CharCode[0] of a string built from -(2^64+1) must be 0');
}

function overflowSingleCasesTest() {
    const nan: number = NaN;
    arktest.assertEQ(string.fromCharCode(nan), '\u{0}', 'CharCode[0] of a string built from NaN must be 0');
    const infin: number = Infinity;
    arktest.assertEQ(string.fromCharCode(infin), '\u{0}', 'CharCode[0] of a string built from Infinity must be 0');
    arktest.assertEQ(string.fromCharCode(9007199254740992.0 as number), '\u{0}', 'CharCode[0] of a string built from 2^53 must be 0');
    arktest.assertEQ(string.fromCharCode(9007199254740991.0 as number), '\u{ffff}',
        'CharCode[0] of a string built from 2^53-1 must be 65535');
    arktest.assertEQ(string.fromCharCode(-9007199254740992.0 as number), '\u{0}',
        'CharCode[0] of a string built from -2^53 must be 0');
    arktest.assertEQ(string.fromCharCode(-9007199254740991.0 as number), '\u{1}',
        'CharCode[0] of a string built from -(2^53-1) must be 1');
    arktest.assertEQ(string.fromCharCode(-9007199254740993.0 as number), '\u{0}',
        'CharCode[0] of a string built from -(2^53+1) must be 0');
    arktest.assertEQ(string.fromCharCode(18446744073709551615.0 as number), '\u{0}',
        'CharCode[0] of a string built from 2^64-1 must be 0');
    arktest.assertEQ(string.fromCharCode(18446744073709551616.0 as number), '\u{0}',
        'CharCode[0] of a string built from 2^64 must be 0');
    arktest.assertEQ(string.fromCharCode(-18446744073709551616.0 as number), '\u{0}',
        'CharCode[0] of a string built from -2^64 must be 0');
    arktest.assertEQ(string.fromCharCode(18446744073709551617.0 as number), '\u{0}',
        'CharCode[0] of a string built from 2^64+1 must be 0');
    arktest.assertEQ(string.fromCharCode(-18446744073709551617.0 as number), '\u{0}',
        'CharCode[0] of a string built from -(2^64+1) must be 0');
}

function stringFromCpsSeqTest() {
    const cpsGolden = '\u{c}\u{7545}\u{ea7e}\u{15fb7}\u{1d4f0}\u{24a29}\u{2bf62}\u{3349b}\u{3a9d4}\u{41f0d}\u{49446}\u{5097f}\u{57eb8}\u{5f3f1}\u{6692a}\u{6de63}\u{7539c}\u{7c8d5}\u{83e0e}\u{8b347}\u{92880}\u{99db9}\u{a12f2}\u{a882b}\u{afd64}\u{b729d}\u{be7d6}\u{c5d0f}\u{cd248}\u{d4781}\u{dbcba}\u{e31f3}\u{ea72c}\u{f1c65}\u{f919e}\u{1006d7}\u{107c10}\u{10f149}';
    let cpsActual = '';
    for (let i = 12; i < 0x10FFFF; i += 30009) {
        if (Char.codeUnitsToEncode(i) == 1) {
            cpsActual += String.fromCharCode(i as number);
        } else {
            cpsActual += String.fromCharCode(Char.getHighSurrogate(i) as number);
            cpsActual += String.fromCharCode(Char.getLowSurrogate(i) as number);
        }
    }
    arktest.assertEQ(cpsActual, cpsGolden, 'cpsActual must be equal to cpsGolden');
}

function heapVerificationTest() {
    arktest.assertEQ(string.fromCharCode(127, NaN), '\u{007f}\u{0}', 'CharCode[0] of a string built from 127, NaN must be 127');
    arktest.assertEQ(string.fromCharCode(127, NaN).length, 2);
    arktest.assertEQ(string.fromCharCode(128, NaN), '\u{0080}\u{0}', 'CharCode[0] of a string built from 128, NaN must be 128');
    arktest.assertEQ(string.fromCharCode(128, NaN).length, 2);
    arktest.assertEQ(string.fromCharCode(-1, NaN), '\u{ffff}\u{0}', 'CharCode[0] of a string built from -1, NaN must be 65535');
    arktest.assertEQ(string.fromCharCode(-1, NaN).length, 2);
    arktest.assertEQ(string.fromCharCode(-100, NaN, Infinity), 'ﾜ\u{0}\u{0}', "Error! Must be 'ﾜ\u{0}\u{0}'");
    arktest.assertEQ(string.fromCharCode(-100, NaN, Infinity).length, 3);
    arktest.assertEQ(string.fromCharCode(-100, NaN), 'ﾜ\u{0}', "Error! Must be 'ﾜ\u{0}'");
    arktest.assertEQ(string.fromCharCode(-100, NaN).length, 2);
    arktest.assertEQ(string.fromCharCode(-100, Infinity), 'ﾜ\u{0}', "Error! Must be 'ﾜ\u{0}'");
    arktest.assertEQ(string.fromCharCode(-100, Infinity).length, 2);
    arktest.assertEQ(string.fromCharCode(65436, NaN), 'ﾜ\u{0}', "Error! Must be 'ﾜ\u{0}'");
    arktest.assertEQ(string.fromCharCode(65436, NaN).length, 2);
    arktest.assertEQ(string.fromCharCode(0xffff0066, -100, NaN, Infinity), 'fﾜ\u{0}\u{0}', "Error! Must be 'fﾜ\u{0}\u{0}'");
    arktest.assertEQ(string.fromCharCode(0xffff0066, -100, NaN, Infinity).length, 4);
    arktest.assertEQ(string.fromCharCode(65535, NaN), '\u{ffff}\u{0}', 'CharCode[0] of a string built from 65535, NaN must be 65535');
    arktest.assertEQ(string.fromCharCode(65535, NaN).length, 2);
    arktest.assertEQ(string.fromCharCode(65536, NaN), '\u{0}\u{0}', 'CharCode[0] of a string built from 65536, NaN must be 0');
    arktest.assertEQ(string.fromCharCode(65536, NaN).length, 2);
    arktest.assertEQ(string.fromCharCode(65435, NaN), 'ﾛ\u{0}', "Error! Must be 'ﾛ\u{0}'");
    arktest.assertEQ(string.fromCharCode(65435, NaN).length, 2);
    arktest.assertEQ(string.fromCharCode(65437, NaN), 'ﾝ\u{0}', "Error! Must be 'ﾝ\u{0}'");
    arktest.assertEQ(string.fromCharCode(65437, NaN).length, 2);
    arktest.assertEQ(string.fromCharCode(126, 126, 125, 128, NaN).length, 5);
    arktest.assertEQ(string.fromCharCode(126, 126, 125, 128, 127, NaN).length, 6);
}

function stringFromCharVarTest() {
    let alpha: char = 0x3B1;
    let beta: char = 0x3B2;
    arktest.assertEQ(string.fromCharCode(alpha, beta), 'αβ', "Must be 'αβ'");
    arktest.assertEQ(string.fromCharCode(Char.toDouble(alpha), Char.toDouble(beta)), 'αβ', "Must be 'αβ'");
}

function stringFromSingleCharVarTest() {
    let beta: char = 0x3B2;
    arktest.assertEQ(string.fromCharCode(beta), 'β', "Must be 'β'");
    arktest.assertEQ(string.fromCharCode(Char.toDouble(beta)), 'β', "Must be 'β'");
}

function stringFromCompositeSymbolTest() {
    arktest.assertEQ(string.fromCharCode(0xd83c, 0xdf03), '🌃');
    arktest.assertEQ(string.fromCharCode(0xd842, 0xdfb7), '𠮷');
}

function main(): int {
    let fromCharCodeTestsuite = new arktest.ArkTestsuite('intrinsics.string_from_char_code');
    fromCharCodeTestsuite.addTest('EmptyString', emptyStringTest);
    fromCharCodeTestsuite.addTest('CompressibleString', compressibleStringTest);
    fromCharCodeTestsuite.addTest('CompressibleSingleString', compressibleSingleStringTest);
    fromCharCodeTestsuite.addTest('IncompressibleString', incompressibleStringTest);
    fromCharCodeTestsuite.addTest('IncompressibleSingleString', incompressibleSingleStringTest);
    fromCharCodeTestsuite.addTest('FalsePositiveWrongSymbolFlag', falsePositiveWrongSymbolFlagTest);
    fromCharCodeTestsuite.addTest('FalsePositiveWrongSymbolFlagSingle', falsePositiveWrongSymbolFlagSingleTest);
    fromCharCodeTestsuite.addTest('NegativeCases', negativeCasesTest);
    fromCharCodeTestsuite.addTest('NegativeSingleCases', negativeSingleCasesTest);
    fromCharCodeTestsuite.addTest('SaturationCases', saturationCasesTest);
    fromCharCodeTestsuite.addTest('SaturationSingleCases', saturationSingleCasesTest);
    fromCharCodeTestsuite.addTest('OverflowCases', overflowCasesTest);
    fromCharCodeTestsuite.addTest('OverflowSingleCases', overflowSingleCasesTest);
    fromCharCodeTestsuite.addTest('StringFromCpsSeq', stringFromCpsSeqTest);
    fromCharCodeTestsuite.addTest('HeapVerification', heapVerificationTest);
    fromCharCodeTestsuite.addTest('StringFromCharVar', stringFromCharVarTest);
    fromCharCodeTestsuite.addTest('StringFromSingleCharVar', stringFromSingleCharVarTest);
    fromCharCodeTestsuite.addTest('StringFromCompositeSymbol', stringFromCompositeSymbolTest);
    return fromCharCodeTestsuite.run();
}