/**
  * Copyright (c) 2026 Huawei Device Co., Ltd.
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

const MAX_U8_VALUE: int = 0xFF;

class NullPtrTrick {
    private p: string
    private leak() { return this.p }
    public constructor() { this.p = this.leak() }
    public static nullptr(): string { return (new NullPtrTrick()).p }
}

interface CharToHexItem {
    ch: char;
    hex: string;
}

const charToHexTable: CharToHexItem[] = [
    { ch: c' ', hex: '20' },
    { ch: c'!', hex: '21' },
    { ch: c'"', hex: '22' },
    { ch: c'#', hex: '23' },
    { ch: c'$', hex: '24' },
    { ch: c'%', hex: '25' },
    { ch: c'&', hex: '26' },
    { ch: c'\'', hex: '27' },
    { ch: c'(', hex: '28' },
    { ch: c')', hex: '29' },
    { ch: c'*', hex: '2a' },
    { ch: c'+', hex: '2b' },
    { ch: c',', hex: '2c' },
    { ch: c'-', hex: '2d' },
    { ch: c'.', hex: '2e' },
    { ch: c'/', hex: '2f' },
    { ch: c'0', hex: '30' },
    { ch: c'1', hex: '31' },
    { ch: c'2', hex: '32' },
    { ch: c'3', hex: '33' },
    { ch: c'4', hex: '34' },
    { ch: c'5', hex: '35' },
    { ch: c'6', hex: '36' },
    { ch: c'7', hex: '37' },
    { ch: c'8', hex: '38' },
    { ch: c'9', hex: '39' },
    { ch: c':', hex: '3a' },
    { ch: c';', hex: '3b' },
    { ch: c'<', hex: '3c' },
    { ch: c'=', hex: '3d' },
    { ch: c'>', hex: '3e' },
    { ch: c'?', hex: '3f' },
    { ch: c'@', hex: '40' },
    { ch: c'A', hex: '41' },
    { ch: c'B', hex: '42' },
    { ch: c'C', hex: '43' },
    { ch: c'D', hex: '44' },
    { ch: c'E', hex: '45' },
    { ch: c'ÿ', hex: 'ff' },
    { ch: c'þ', hex: 'fe' },
    { ch: c'ý', hex: 'fd' },
    { ch: c'ü', hex: 'fc' },
    { ch: c'û', hex: 'fb' },
    { ch: c'ú', hex: 'fa' },
    { ch: c'ù', hex: 'f9' },
    { ch: c'ø', hex: 'f8' },
    { ch: c'÷', hex: 'f7' },
    { ch: c'ö', hex: 'f6' },
    { ch: c'õ', hex: 'f5' },
    { ch: c'ô', hex: 'f4' },
    { ch: c'ó', hex: 'f3' },
    { ch: c'ò', hex: 'f2' },
    { ch: c'ñ', hex: 'f1' },
    { ch: c'ð', hex: 'f0' },
    { ch: c'А', hex: '410' },
    { ch: c'Б', hex: '411' },
    { ch: c'В', hex: '412' },
    { ch: c'Г', hex: '413' },
    { ch: c'Д', hex: '414' },
    { ch: c'ሴ', hex: '1234' },
    { ch: c'噸', hex: '5678' },
    { ch: c'退', hex: '9000' },
]

function testCharToHex() {
    for (let item: CharToHexItem of charToHexTable) {
        let hex = item.ch.toHex();
        arktest.assertEQ(hex, item.hex,
            `Char(${item.ch}).toHex() should be '${item.hex}', got '${hex}'`)
        hex = item.ch.toString(16);
        arktest.assertEQ(hex, item.hex,
            `Char(${item.ch}).toHex() should be '${item.hex}', got '${hex}'`)
    }
}

function testCharFromHex() {
    let nullPointerErrors: int = 0;
    let illegalArgumentErrors: int = 0;

    for (let item: CharToHexItem of charToHexTable) {
        try {
            const ch = Char.fromHex(item.hex, 0, item.hex.getLength());
            arktest.assertEQ(ch, item.ch,
                `Char.fromHex(${item.hex}) should be '${item.ch}', got '${ch}'`)
        } catch (e) {
            if (e instanceof NullPointerError) {
                ++nullPointerErrors;
            } else if (e instanceof IllegalArgumentError) {
                ++illegalArgumentErrors;
            } else {
                throw e;
            }
        }
    }
    arktest.assertEQ(nullPointerErrors, 0, 'NullPointerError count must be zero');
    arktest.assertEQ(illegalArgumentErrors, 0, 'IllegalArgumentError count must be zero');

    const s = '2a 42E 05a 31 7000 9000';
    arktest.assertEQ(Char.fromHex(s, s.indexOf(c'2'), 2), c'*');
    arktest.assertEQ(Char.fromHex(s, s.indexOf(c'4'), 3), c'Ю');
    arktest.assertEQ(Char.fromHex(s, s.indexOf(c'0'), 3), c'Z');
    arktest.assertEQ(Char.fromHex(s, s.indexOf(c'3'), 2), c'1');
    arktest.assertEQ(Char.fromHex(s, s.indexOf(c'7')), c'瀀');
    arktest.assertEQ(Char.fromHex(s, s.indexOf(c'9')), c'退');

    arktest.assertEQ(Char.fromHex('from', 0, 1), Char.fromHex('f', 0, 1));
    arktest.assertEQ(Char.fromHex('ffom', 0, 2), Char.fromHex('ff', 0, 2));
    arktest.assertEQ(Char.fromHex('fffm', 0, 3), Char.fromHex('fff', 0, 3));

    const s1 = 'Загадка: \\u0410 \\u0438 \\u0411 сидели на трубе';
    let i = s1.indexOf('\\u');
    arktest.assertEQ(Char.fromHex(s1, i + 2), c'А');
    i = s1.indexOf('\\u', i + 2);
    arktest.assertEQ(Char.fromHex(s1, i + 2), c'и');
    i = s1.indexOf('\\u', i + 2);
    arktest.assertEQ(Char.fromHex(s1, i + 2), c'Б');

    const uriEncoded = 'A%3ABC%3BD%26EF';  // URI: 'A:BC;D&EF';
    i = uriEncoded.indexOf('%');
    arktest.assertEQ(Char.fromHex(uriEncoded, i + 1, 2), c':');
    i = uriEncoded.indexOf('%', i + 1);
    arktest.assertEQ(Char.fromHex(uriEncoded, i + 1, 2), c';');
    i = uriEncoded.indexOf('%', i + 1);
    arktest.assertEQ(Char.fromHex(uriEncoded, i + 1, 2), c'&');
}

// If default arguments are used then Char.fromHex() must be the inversion of Char.toHex() (and vice versa):
//
//   Char.fromHex(Char.toHex(ch)) == ch
//   Char.toHex(Char.fromHex(hex)) == hex
//
function testInversion() {
    for (let item: CharToHexItem of charToHexTable) {
        const ch = Char.fromHex(item.ch.toHex());
        arktest.assertEQ(ch, item.ch, `Char.fromHex(Char(${item.ch}).toHex()) should be '${item.ch}', got '${ch}'`)
        const hex = Char.fromHex(item.hex).toHex();
        arktest.assertEQ(hex, item.hex, `Char.fromHex(${item.hex}).toHex() should be '${item.hex}', got '${hex}'`)
    }
}

function testToHexSpecification() {
    let c1 = new Char(c'!');
    // Prefix '0x' must not be added to the resulting hex string
    arktest.assertEQ(c1.toHex(), '21');
    // Negative padding is reset to zero
    arktest.assertEQ(c1.toHex(-1, c'0'), '21');
    // Check positive padding
    arktest.assertEQ(c1.toHex(6, c'0'), '00000021');
    // Check utf16 pad char
    arktest.assertEQ(c1.toHex(1, c'Ж'), 'Ж21');
    // Check '\u0000'
    let nul = new Char();
    arktest.assertEQ(nul.toHex(1, c'-'), '-0');
}

function testFromHexSpecification() {

    // There must be no '0x','0X' prefixes in hex string
    let hex = '0x65';
    try {
        let c = Char.fromHex(hex);
    } catch (e) {
        arktest.assertTrue(e instanceof IllegalArgumentError, 'expected IllegalArgumentError');
    }

    // Hex string must contain '0123456789AbBbCcDdEeFf' only
    let illegalArgumentErrors = 0;
    const allowed = '0123456789AbBbCcDdEeFf';
    for (let i = 1; i <= MAX_U8_VALUE; ++i) {
        let c: char = i.toChar();
        if (allowed.indexOf(c) == -1) {
            try {
                Char.fromHex('ABC' + (new String(c)), 0, 4);
            } catch (e) {
                if (e instanceof IllegalArgumentError) {
                    ++illegalArgumentErrors;
                }
            }
        }
    }
    const expectedErrors = MAX_U8_VALUE - allowed.getLength();
    arktest.assertEQ(illegalArgumentErrors, expectedErrors, 'expects ' + expectedErrors + ' IllegalArgumentError');

    // Accepts both lower and upper cases 'AaBbCcDdEeFf'
    arktest.assertEQ(Char.fromHex('Aa', 0, 2), c'\u00AA', 'Char.fromHex should be case independent');
    arktest.assertEQ(Char.fromHex('Bb', 0, 2), c'\u00BB', 'Char.fromHex should be case independent');
    arktest.assertEQ(Char.fromHex('Cc', 0, 2), c'\u00CC', 'Char.fromHex should be case independent');
    arktest.assertEQ(Char.fromHex('Dd', 0, 2), c'\u00DD', 'Char.fromHex should be case independent');
    arktest.assertEQ(Char.fromHex('Ee', 0, 2), c'\u00EE', 'Char.fromHex should be case independent');
    arktest.assertEQ(Char.fromHex('Ff', 0, 2), c'\u00FF', 'Char.fromHex should be case independent');

    // Empty hex string results into c'\u0000'
    hex = '';
    arktest.assertEQ(Char.fromHex(hex), c'\u0000', 'expects c\'\\u0000\'');

    const lineHex = 'aaaabbbbccccdddd9876';
    // Test Tree hex strings
    arktest.assertEQ(Char.fromHex(lineHex + '0' + '1'), Char.fromHex('aaaa'));
    arktest.assertEQ(Char.fromHex(lineHex + '0' + '2', -1), Char.fromHex('aaaa'));
    arktest.assertEQ(Char.fromHex(lineHex + '0' + '3', -100, -1), Char.fromHex('aaaa'));
    arktest.assertEQ(Char.fromHex(lineHex + '0' + '4', 1), Char.fromHex('aaab'));
    arktest.assertEQ(Char.fromHex(lineHex + '0' + '5', 1, -1), Char.fromHex('aaab'));
    arktest.assertEQ(Char.fromHex(lineHex + '0' + '5', 1, -100), Char.fromHex('aaab'));
    arktest.assertEQ(Char.fromHex(lineHex + '0' + '6', 1, 0), Char.fromHex('aaab'));
    arktest.assertEQ(Char.fromHex(lineHex + '0' + '7', 1, 1), Char.fromHex('a'));
    arktest.assertEQ(Char.fromHex(lineHex + '0' + '8', 1, 2), Char.fromHex('aa'));
    arktest.assertEQ(Char.fromHex(lineHex + '0' + '9', 1, 3), Char.fromHex('aaa'));
    arktest.assertEQ(Char.fromHex(lineHex + '0' + '9', 20), Char.fromHex('9'));
    arktest.assertEQ(Char.fromHex(lineHex + '1' + '9', 20), Char.fromHex('19'));
    arktest.assertEQ(Char.fromHex(lineHex + '1' + '9', 20, 1), Char.fromHex('1'));
    arktest.assertEQ(Char.fromHex(lineHex + '1' + '9', 20, 2), Char.fromHex('19'));
    arktest.assertEQ(Char.fromHex(lineHex + '1' + '9', 21, 1), Char.fromHex('9'));

    // Test Sliced hex strings
    arktest.assertEQ(Char.fromHex(lineHex.substring(-1)), Char.fromHex('aaaa'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(0)), Char.fromHex('aaaa'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(1)), Char.fromHex('aaab'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(2)), Char.fromHex('aabb'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(3)), Char.fromHex('abbb'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(5)), Char.fromHex('bbbc'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(7)), Char.fromHex('bccc'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(11)), Char.fromHex('cddd'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(17)), Char.fromHex('876'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(19)), Char.fromHex('6'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(20)), c'\u0000');
    arktest.assertEQ(Char.fromHex(lineHex.substring(1), 1), Char.fromHex('aabb'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(1), 1, 0), Char.fromHex('aabb'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(1), 1, -1), Char.fromHex('aabb'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(1), 2), Char.fromHex('abbb'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(1), 3), Char.fromHex('bbbb'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(2), 1, 2), Char.fromHex('ab'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(4), -1, 0), Char.fromHex('bbbb'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(4), -1), Char.fromHex('bbbb'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(4), -100), Char.fromHex('bbbb'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(4), -1, -1), Char.fromHex('bbbb'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(4), -100, -100), Char.fromHex('bbbb'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(4), 13), Char.fromHex('876'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(4), 14), Char.fromHex('76'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(4), 15), Char.fromHex('6'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(4), 2, 4), Char.fromHex('bbcc'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(4), 12, 3), Char.fromHex('987'));
    arktest.assertEQ(Char.fromHex(lineHex.substring(4), 12, 4), Char.fromHex('9876'));

    let outOfBoundsErrors: int = 0;
    try {
        Char.fromHex(lineHex.substring(4), 16);
    } catch (e) {
        if (e instanceof StringIndexOutOfBoundsError) {
            outOfBoundsErrors += 1;
        }
    }
    arktest.assertEQ(outOfBoundsErrors, 1);

    try {
        Char.fromHex(lineHex.substring(4), 13, 4);
    } catch (e) {
        if (e instanceof StringIndexOutOfBoundsError) {
            outOfBoundsErrors += 1;
        }
    }
    arktest.assertEQ(outOfBoundsErrors, 2);

    // -----------------------------------
    // Test 'startIndex' and 'length' args
    // -----------------------------------

    // If the length is default and startIndex > 0
    hex = "R0401";
    let yo = Char.fromHex(hex, 1);
    arktest.assertEQ(yo, c'Ё');
    yo = Char.fromHex(hex, 2);
    arktest.assertEQ(yo, c'Ё');

    // If startIndex < 0 then it is reset to 0.
    hex = "0401";
    yo = Char.fromHex(hex, -1);
    arktest.assertEQ(yo, c'Ё');

    // if startIndex >= hex.getLength() then throw StringIndexOutOfBoundsError
    outOfBoundsErrors = 0;
    for (let i = -1; i <= 1; ++i) {
        try {
            yo = Char.fromHex(hex, 4, i);
        } catch (e) {
            if (e instanceof StringIndexOutOfBoundsError) {
                outOfBoundsErrors += 1;
            }
        }
    }
    arktest.assertEQ(outOfBoundsErrors, 3);

    // if length < 0 then assume it is 0
    yo = Char.fromHex(hex, 0, -1);
    arktest.assertEQ(yo, c'Ё');

    // if length > 4 then assume it is 4
    yo = Char.fromHex(hex, 0, 5);
    arktest.assertEQ(yo, c'Ё');

    // if length > 0 && (startIndex + length) > hex.getLength() then throw StringIndexOutOfBoundsError
    outOfBoundsErrors = 0;
    for (let i = 1, j = 4; i < 5; ++i, --j) {
        try {
            yo = Char.fromHex(hex, i, j);
        } catch (e) {
            if (e instanceof StringIndexOutOfBoundsError) {
                outOfBoundsErrors += 1;
            }
        }
    }
    arktest.assertEQ(outOfBoundsErrors, 4);

    // if length == 0 then convert up to 4 chars until non-convertible char (if any) is encountered
    // by default length is 0
    arktest.assertEQ(Char.fromHex('fro'), Char.fromHex('f', 0, 1));
    arktest.assertEQ(Char.fromHex('ffo'), Char.fromHex('ff', 0, 2));
    arktest.assertEQ(Char.fromHex('fff'), Char.fromHex('fff', 0, 3));
    arktest.assertEQ(Char.fromHex('from'), Char.fromHex('f', 0, 1));
    arktest.assertEQ(Char.fromHex('ffom'), Char.fromHex('ff', 0, 2));
    arktest.assertEQ(Char.fromHex('fffm'), Char.fromHex('fff', 0, 3));

    // explicitly set length to 0
    arktest.assertEQ(Char.fromHex('from', 0, 0), Char.fromHex('f', 0, 1));

    // NullPointerError
    let nullPointerErrors = 0;
    try {
        Char.fromHex(NullPtrTrick.nullptr());
    } catch (e) {
        if (e instanceof NullPointerError) {
            nullPointerErrors += 1;
        }
    }
    arktest.assertEQ(nullPointerErrors, 1, 'expected NullPointerError');
}

function main(): int {
    let suite = new arktest.ArkTestsuite('CharHexConv');
    suite.addTest('testToHexSpecification', testToHexSpecification);
    suite.addTest('testFromHexSpecification', testFromHexSpecification);
    suite.addTest('testCharToHex', testCharToHex);
    suite.addTest('testCharFromHex', testCharFromHex);
    suite.addTest('testInversion', testInversion);
    return suite.run();
}

