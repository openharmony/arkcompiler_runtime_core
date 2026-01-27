/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

let Empty: String = '';
let OneChar: String = 'x';
let EscChars: String = '\f\n\r\t\v';
let RusChars: String = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя'; // 16-bit
let EngChars: String = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz';               //  8-bit
let MixedChars: String = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнAaBbCcDdEeFfGgHhIiJjKkLl';           // 16-bit
let DupChars: String = 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA';               //  8-bit
let Digits: String = '0123456789';
let Symbols: String = '`~!@#$%^&*()_-+={}[]:;\"\'\\|/<>,.';
const CharNotFound: int = -1;

function testAllSubstrings(str: String, strict: boolean = true) {
    for (let i = strict ? 0 : 1; i < str.getLength(); ++i) {
        arktest.assertEQ(str.indexOf(str.substring(i, str.length), i - 1), strict ? i : i - 1);
    }
    for (let i = 0; i < str.getLength(); ++i) {
        arktest.assertEQ(str.indexOf(str.substring(0, i), 0), 0);
    }
}

function testEmptyString() {
    arktest.assertEQ(Empty.indexOf(''.concat('x'), 0), CharNotFound);
    arktest.assertEQ(Empty.indexOf(''.concat(''), 0), 0);
    arktest.assertEQ(Empty.indexOf('', 0), 0);
    arktest.assertEQ(Empty.indexOf('x', 0), CharNotFound);
}

function testTreeOneCharString() {
    arktest.assertEQ(OneChar.indexOf(''.concat('x'), 0), 0);
    arktest.assertEQ(OneChar.indexOf('x', 0), 0);
    arktest.assertEQ(OneChar.indexOf('x', -1), 0);
    arktest.assertEQ(OneChar.indexOf('xx', 0), CharNotFound);
    arktest.assertEQ(OneChar.indexOf('xi', 0), CharNotFound);
    arktest.assertEQ(OneChar.indexOf(''.concat('l'), 0), CharNotFound);
    arktest.assertEQ(OneChar.indexOf('l', 0), CharNotFound);
    arktest.assertEQ(OneChar.indexOf('hi', 0), CharNotFound);
    arktest.assertEQ(OneChar.indexOf('h'.concat('k'), 0), CharNotFound);
}

function testShortLinearString() {
    let str = "AaBb";
    arktest.assertEQ(str.indexOf('AaBb'), 0);
    arktest.assertEQ(str.indexOf('AaBb', -1), 0);
    arktest.assertEQ(str.indexOf('ABBb'), CharNotFound);
    arktest.assertEQ(str.indexOf('aBb'), 1);
    arktest.assertEQ(str.indexOf('aBb', -1), 1);
    arktest.assertEQ(str.indexOf('aBb', 1), 1);
    arktest.assertEQ(str.indexOf('aBb', 2), CharNotFound);
    arktest.assertEQ(str.indexOf('aXb'), CharNotFound);
    arktest.assertEQ(str.indexOf('b'), 3);
    arktest.assertEQ(str.indexOf('b', -1), 3);
    arktest.assertEQ(str.indexOf('b', 3), 3);
    arktest.assertEQ(str.indexOf('b', 4), CharNotFound);

    str = "AфыBы";
    arktest.assertEQ(str.indexOf('AфыBы'), 0);
    arktest.assertEQ(str.indexOf('AЙыBb'), CharNotFound);
    arktest.assertEQ(str.indexOf('AфыB'), 0);
    arktest.assertEQ(str.indexOf('AфыB', -1), 0);
    arktest.assertEQ(str.indexOf('AфыB', 1), CharNotFound);
    arktest.assertEQ(str.indexOf('фыB'), 1);
    arktest.assertEQ(str.indexOf('фыB', -1), 1);
    arktest.assertEQ(str.indexOf('фыB', 1), 1);
    arktest.assertEQ(str.indexOf('фыB', 2), CharNotFound);
    arktest.assertEQ(str.indexOf('фыB', 3), CharNotFound);
    arktest.assertEQ(str.indexOf('ф'), 1);
    arktest.assertEQ(str.indexOf('B'), 3);
    arktest.assertEQ(str.indexOf('B', 3), 3);
    arktest.assertEQ(str.indexOf('B', 4), CharNotFound);
    arktest.assertEQ(str.indexOf('Bы'), 3);
    arktest.assertEQ(str.indexOf('Bы', 3), 3);
    arktest.assertEQ(str.indexOf('Бы'), CharNotFound);
    arktest.assertEQ(str.indexOf('Ab'), CharNotFound);
    arktest.assertEQ(str.indexOf('ы'), 2);
    arktest.assertEQ(str.indexOf('ы', -1), 2);
    arktest.assertEQ(str.indexOf('ы', 3), 4);
    arktest.assertEQ(str.indexOf('ы', 4), 4);
    arktest.assertEQ(str.indexOf('ы', 5), CharNotFound);
}

function testEscapedLinearString() {
    arktest.assertEQ(EscChars.indexOf('\r\t\v\t', 1), CharNotFound);
    arktest.assertEQ(EscChars.indexOf('\r\t\v', 1), 2);
    arktest.assertEQ(EscChars.indexOf('\r\t\v', -1), 2);
    arktest.assertEQ(EscChars.indexOf('\r\t\v', 2), 2);
    arktest.assertEQ(EscChars.indexOf('\r\t\v', 3), CharNotFound);
    arktest.assertEQ(EscChars.indexOf(EscChars), 0);
}

function testEscapedLinearSubstring() {
    testAllSubstrings(EscChars);
}

function testEscapedTreeSubstring() {
    let mixedTreeChars = EscChars.concat(EscChars);
    testAllSubstrings(mixedTreeChars);
}

function testEscapedMixedTreeSubstring() {
    let mixedTreeChars = EscChars.concat(EngChars);
    testAllSubstrings(mixedTreeChars);
}

function testEscapedSlicedSubstring() {
    let escSlicedChars = EscChars.substring(1, 20);
    testAllSubstrings(escSlicedChars);
}

function testNonCompressedLinearString() {
    arktest.assertEQ(RusChars.indexOf('Ѫ'), CharNotFound);
    arktest.assertEQ(RusChars.indexOf(RusChars), 0);
    arktest.assertEQ(RusChars.indexOf(RusChars, 0), 0);

    arktest.assertEQ(RusChars.indexOf(RusChars, 1), CharNotFound);

    let RusCharsTooLongKey = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя_';
    arktest.assertEQ(RusChars.indexOf(RusCharsTooLongKey), CharNotFound);

    let rusChars_1 = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let rusChars_2 = 'ОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя';
    arktest.assertEQ(RusChars.indexOf(rusChars_1), 0);
    arktest.assertEQ(RusChars.indexOf(rusChars_2), rusChars_1.length);
    arktest.assertEQ(RusChars.indexOf(rusChars_2, rusChars_1.length), rusChars_1.length);
}

function testNonCompressedTreeString() {
    let rusChars_1 = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let rusChars_2 = 'ОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя';
    let rusTreeChars = rusChars_1.concat(rusChars_2);
    arktest.assertTrue(rusTreeChars === RusChars);
    arktest.assertTrue(RusChars === rusTreeChars);
    arktest.assertEQ(rusTreeChars.indexOf(rusChars_1), 0);
    arktest.assertEQ(rusTreeChars.indexOf(rusChars_2), rusChars_1.length);
    arktest.assertEQ(rusTreeChars.indexOf(rusChars_2, rusChars_1.length), rusChars_1.length);
    arktest.assertEQ(RusChars.indexOf(rusTreeChars), 0);
    arktest.assertEQ(RusChars.indexOf(rusTreeChars, -1), 0);
    arktest.assertEQ(RusChars.indexOf(rusTreeChars, 0), 0);
    arktest.assertEQ(RusChars.indexOf(rusTreeChars, 1), CharNotFound);
}

function testNonCompressedSlicedString() {
    let rusChars_1 = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let rusChars_2 = 'ОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя';
    let rusTreeChars = rusChars_1.concat(rusChars_2);
    let rusSlicedChars = rusTreeChars.substring(3, 48);

    arktest.assertEQ(RusChars.indexOf(rusSlicedChars), 3);
    arktest.assertEQ(rusSlicedChars.indexOf(rusSlicedChars), 0);
    arktest.assertEQ(rusSlicedChars.indexOf(rusSlicedChars, 0), 0);
    arktest.assertEQ(rusSlicedChars.indexOf(rusSlicedChars, 1), CharNotFound);

    let rusCharsKey_1 = 'бВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    arktest.assertEQ(rusSlicedChars.indexOf(rusCharsKey_1), 0);
    arktest.assertEQ(rusSlicedChars.indexOf(rusCharsKey_1, 0), 0);
    arktest.assertEQ(rusSlicedChars.indexOf(rusCharsKey_1, 1), CharNotFound);
    arktest.assertEQ(rusCharsKey_1, rusChars_1.substring(3, rusChars_1.length));
    arktest.assertEQ(rusSlicedChars.indexOf(rusChars_1.substring(3, rusChars_1.length)), 0);

    let rusCharsKey_2 = 'ОоПпРрСсТтУуФфХхЦц';
    arktest.assertEQ(rusCharsKey_2, rusChars_2.substring(0, 18));
    arktest.assertEQ(rusSlicedChars.indexOf(rusCharsKey_2), rusCharsKey_1.length);
    arktest.assertEQ(rusSlicedChars.indexOf(rusCharsKey_2, rusCharsKey_1.length), rusCharsKey_1.length);
    arktest.assertEQ(rusSlicedChars.indexOf(rusCharsKey_2, rusCharsKey_1.length - 1), rusCharsKey_1.length);
    arktest.assertEQ(rusSlicedChars.indexOf(rusCharsKey_2, rusCharsKey_1.length + 1), CharNotFound);

    let rusTreeCharsTooLongKey = rusChars_1.concat(rusChars_2).concat("X");
    let rusSlicedCharsTooLongKey = rusTreeCharsTooLongKey.substring(3, 49);
    arktest.assertEQ(rusSlicedCharsTooLongKey.length, 46);
    arktest.assertEQ(rusSlicedChars.indexOf(rusSlicedCharsTooLongKey), CharNotFound);
}

function testNonCompressedLinearSubstring() {
    testAllSubstrings(RusChars);
}

function testNonCompressedTreeSubstring() {
    let rusChars_1 = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let rusChars_2 = 'ОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя';
    let rusTreeChars = rusChars_1.concat(rusChars_2);
    testAllSubstrings(rusTreeChars);
}

function testNonCompressedSlicedSubstring() {
    let rusChars_1 = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let rusChars_2 = 'ОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя';
    let rusTreeChars = rusChars_1.concat(rusChars_2);
    let rusSlicedChars = rusTreeChars.substring(3, 30);
    testAllSubstrings(rusSlicedChars);
}

function testCompressedLinearString() {
    arktest.assertEQ(EngChars.indexOf(c'æ'),CharNotFound);
    arktest.assertEQ(EngChars.indexOf('æ'), CharNotFound);
    arktest.assertEQ(EngChars.indexOf(EngChars), 0);
    arktest.assertEQ(EngChars.indexOf(EngChars, 1), CharNotFound);

    let EngCharsTooLongKey = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz_';
    arktest.assertEQ(EngChars.indexOf(EngCharsTooLongKey), CharNotFound);
}

function testCompressedTreeString() {
    let engChars_1 = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNn';
    let engChars_2 = 'OoPpQqRrSsTtUuVvWwXxYyZz';
    let engTreeChars = engChars_1.concat(engChars_2);
    arktest.assertTrue(engTreeChars === EngChars);
    arktest.assertTrue(EngChars === engTreeChars);
    arktest.assertEQ(EngChars.indexOf(engChars_1), 0);
    arktest.assertEQ(EngChars.indexOf(engChars_2), engChars_1.length);
    arktest.assertEQ(engTreeChars.indexOf(engChars_1), 0);
    arktest.assertEQ(engTreeChars.indexOf(engChars_2), engChars_1.length);
    arktest.assertEQ(engTreeChars.indexOf(engTreeChars.concat('R')), CharNotFound);
    arktest.assertEQ(engTreeChars.indexOf('R'.concat(engTreeChars)), CharNotFound);
}

function testCompressedSlicedString() {
    let engChars_1 = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNn';
    let engChars_2 = 'OoPpQqRrSsTtUuVvWwXxYyZz';
    let engTreeChars = engChars_1.concat(engChars_2);
    let engSlicedChars = engTreeChars.substring(3, 43);
    arktest.assertEQ(EngChars.indexOf(engSlicedChars), 3);
    arktest.assertEQ(engSlicedChars.indexOf(engSlicedChars), 0);
    arktest.assertEQ(engSlicedChars.indexOf(engSlicedChars, 0), 0);
    arktest.assertEQ(engSlicedChars.indexOf(engSlicedChars, 1), CharNotFound);

    let engCharsKey_1 = 'bCcDdEeFfGgHhIiJjKkLlMmNn';
    arktest.assertEQ(engSlicedChars.indexOf(engCharsKey_1), 0);
    arktest.assertEQ(engSlicedChars.indexOf(engCharsKey_1, 0), 0);
    arktest.assertEQ(engSlicedChars.indexOf(engCharsKey_1, 1), CharNotFound);
    arktest.assertEQ(engCharsKey_1, engChars_1.substring(3, engChars_1.length));
    arktest.assertEQ(engSlicedChars.indexOf(engChars_1.substring(3, engChars_1.length)), 0);

    let engCharsKey_2 = 'OoPpQqRrSsTtUuV';
    arktest.assertEQ(engCharsKey_2, engChars_2.substring(0, 15));
    arktest.assertEQ(engSlicedChars.indexOf(engCharsKey_2), engCharsKey_1.length);
    arktest.assertEQ(engSlicedChars.indexOf(engCharsKey_2, engCharsKey_1.length), engCharsKey_1.length);
    arktest.assertEQ(engSlicedChars.indexOf(engCharsKey_2, engCharsKey_1.length - 1), engCharsKey_1.length);
    arktest.assertEQ(engSlicedChars.indexOf(engCharsKey_2, engCharsKey_1.length + 1), CharNotFound);

    let engTreeCharsTooLongKey = engChars_1.concat(engChars_2).concat("X");
    let engSlicedCharsTooLongKey = engTreeCharsTooLongKey.substring(3, 44);
    arktest.assertEQ(engSlicedCharsTooLongKey.length, 41);
    arktest.assertEQ(engSlicedChars.indexOf(engSlicedCharsTooLongKey), CharNotFound);
}

function testCompressedLinearSubstring() {
    testAllSubstrings(EngChars);
}

function testCompressedTreeSubstring() {
    let engChars_1 = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNn';
    let engChars_2 = 'OoPpQqRrSsTtUuVvWwXxYyZz';
    let engTreeChars = engChars_1.concat(engChars_2);
    testAllSubstrings(engTreeChars);
}

function testCompressedSlicedSubstring() {
    let engChars_1 = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNn';
    let engChars_2 = 'OoPpQqRrSsTtUuVvWwXxYyZz';
    let engTreeChars = engChars_1.concat(engChars_2);
    let engSlicedChars = engTreeChars.substring(3, 30);
    testAllSubstrings(engSlicedChars);
}

function testMixedCompressingLinearString() {
    let rusChars = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let engChars = 'AaBbCcDdEeFfGgHhIiJjKkLl';
    arktest.assertEQ(MixedChars.indexOf(MixedChars), 0);
    arktest.assertEQ(MixedChars.indexOf(MixedChars, 1), CharNotFound);
    arktest.assertEQ(MixedChars.indexOf(rusChars), 0);
    arktest.assertEQ(MixedChars.indexOf(engChars), rusChars.length);

    let rusCharsLongKey = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн!';
    let mixedCharsLongKey = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнAaBbCcDdEeFfGgHhIiJjKkLl!';
    arktest.assertEQ(MixedChars.indexOf(rusCharsLongKey), CharNotFound);
    arktest.assertEQ(MixedChars.indexOf(mixedCharsLongKey), CharNotFound);
}

function testMixedCompressingTreeString() {
    let rusChars = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let engChars = 'AaBbCcDdEeFfGgHhIiJjKkLl';
    let mixedTreeChars = rusChars.concat(engChars);
    arktest.assertTrue(mixedTreeChars === MixedChars);
    arktest.assertTrue(MixedChars === mixedTreeChars);
    arktest.assertEQ(mixedTreeChars.indexOf(rusChars), 0);
    arktest.assertEQ(mixedTreeChars.indexOf(engChars), rusChars.length);

    arktest.assertEQ(mixedTreeChars.indexOf(rusChars, 1), CharNotFound);
    arktest.assertEQ(mixedTreeChars.indexOf(rusChars + '!'), CharNotFound);
    arktest.assertEQ(mixedTreeChars.indexOf(rusChars.concat('!')), CharNotFound);
    arktest.assertEQ(mixedTreeChars.indexOf(engChars + '!'), CharNotFound);
    arktest.assertEQ(mixedTreeChars.indexOf(engChars.concat('!')), CharNotFound);
    arktest.assertEQ(mixedTreeChars.indexOf(mixedTreeChars + '!'), CharNotFound);
    arktest.assertEQ(mixedTreeChars.indexOf(mixedTreeChars.concat('!')), CharNotFound);
}

function testMixedCompressingSlicedString() {
    let rusChars = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let engChars = 'OoPpQqRrSsTtUuVvWwXxYyZz';
    let mixedTreeChars = rusChars.concat(engChars);
    let mixedSlicedChars = mixedTreeChars.substring(3, rusChars.length + 15);
    arktest.assertEQ(mixedSlicedChars.length, rusChars.length + 15 - 3);
    let rusCharsKey = 'бВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    arktest.assertEQ(mixedSlicedChars.indexOf(rusCharsKey), 0);
    arktest.assertEQ(mixedSlicedChars.indexOf(rusCharsKey, 1), CharNotFound);
    arktest.assertEQ(rusCharsKey, rusChars.substring(3, rusChars.length));
    arktest.assertEQ(mixedSlicedChars.indexOf(rusChars.substring(3, rusChars.length)), 0);

    let engCharsKey = 'OoPpQqRrSsTtUuV';
    arktest.assertEQ(engCharsKey, engChars.substring(0, 15));
    arktest.assertEQ(mixedSlicedChars.indexOf(engCharsKey), rusCharsKey.length);
    arktest.assertEQ(mixedSlicedChars.indexOf(engCharsKey, 3), rusCharsKey.length);
    arktest.assertEQ(mixedSlicedChars.indexOf(engCharsKey, rusCharsKey.length), rusCharsKey.length);
    arktest.assertEQ(mixedSlicedChars.indexOf(engCharsKey, rusCharsKey.length + 1), CharNotFound);

    let MixedSlicedTooLongKey = (rusCharsKey.concat(engCharsKey).concat('!!!')).substring(1,
        rusCharsKey.length + engCharsKey.length + 2);
    arktest.assertLT(mixedSlicedChars.length, MixedSlicedTooLongKey.length);
    arktest.assertEQ(mixedSlicedChars.indexOf(MixedSlicedTooLongKey), CharNotFound);
}

function testMixedCompressingLinearSubstring() {
    testAllSubstrings(MixedChars);
}

function testMixedCompressingTreeSubstring() {
    let rusChars = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let engChars = 'AaBbCcDdEeFfGgHhIiJjKkLl';
    let mixedTreeChars = rusChars.concat(engChars);
    testAllSubstrings(mixedTreeChars);
}

function testMixedCompressingSlicedSubstring() {
    let rusChars = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let engChars = 'OoPpQqRrSsTtUuVvWwXxYyZz';
    let mixedTreeChars = rusChars.concat(engChars);
    let mixedSlicedChars = mixedTreeChars.substring(3, rusChars.length + 15);
    testAllSubstrings(mixedSlicedChars);
}

function testDupCharsLinearSubstring() {
    testAllSubstrings(DupChars, false);
}

function testDupCharsTreeSubstring() {
    let mixedTreeChars = DupChars.concat(DupChars);
    testAllSubstrings(mixedTreeChars, false);
}

function testDupCharsSlicedSubstring() {
    let dupSlicedChars = DupChars.substring(1, 26);
    arktest.assertEQ(dupSlicedChars.length, 25);
    testAllSubstrings(dupSlicedChars, false);
}

function testDigitsLinearSubstring() {
    testAllSubstrings(Digits);
}

function testDigitsTreeSubstring() {
    let mixedTreeChars = Digits.concat(Digits);
    testAllSubstrings(mixedTreeChars);
}

function testDigitsSlicedSubstring() {
    let digitsSlicedChars = Digits.substring(1, 8);
    arktest.assertEQ(digitsSlicedChars.length, 7);
    testAllSubstrings(digitsSlicedChars);
}

function testSymbolsLinearString() {
    arktest.assertEQ(Symbols.indexOf(Symbols), 0);
    arktest.assertEQ(Symbols.indexOf(Symbols, 1), CharNotFound);

    let symbolsCharsTooLongKey = '`~!@#$%^&*()_-+={}[]:;\"\'\\|/<>,.!';
    arktest.assertEQ(Symbols.indexOf(symbolsCharsTooLongKey), CharNotFound);
}

function testSymbolsTreeString() {
    let symbols_1 = '`~!@#$%^&*()_-+={}[]';
    let symbols_2 = ':;\"\'\\|/<>,.';
    let symbolsTreeChars = symbols_1.concat(symbols_2);
    arktest.assertEQ(symbolsTreeChars.indexOf(symbolsTreeChars), 0);
    arktest.assertEQ(symbolsTreeChars.indexOf(symbolsTreeChars, 1), CharNotFound);

    let symbolsTreeCharsTooLongKey = symbolsTreeChars.concat('!');
    arktest.assertEQ(symbolsTreeChars.indexOf(symbolsTreeCharsTooLongKey), CharNotFound);
}

function testSymbolsSlicedString() {
    let symbols_1 = '`~!@#$%^&*()_-+={}[]';
    let symbols_2 = ':;\"\'\\|/<>,.';
    let symbolsTreeChars = symbols_1.concat(symbols_2);
    let symbolsSlicedChars = symbolsTreeChars.substring(1, 14);
    arktest.assertEQ(symbolsSlicedChars.indexOf(symbolsSlicedChars), 0);
    arktest.assertEQ(symbolsSlicedChars.indexOf(symbolsSlicedChars, 1), CharNotFound);

    let symbolsTreeCharsTooLongKey = symbolsTreeChars.concat('!!!');
    let symbolsSlicedCharsTooLongKey = symbolsTreeCharsTooLongKey.substring(1, 15);
    arktest.assertEQ(symbolsSlicedCharsTooLongKey.length, 14);
    arktest.assertEQ(symbolsSlicedChars.indexOf(symbolsSlicedCharsTooLongKey), CharNotFound);
}

function testSymbolsLinearSubstring() {
    testAllSubstrings(Symbols);
}

function testSymbolsTreeSubstring() {
    let symbols_1 = '`~!@#$%^&*()_-+={}[]';
    let symbols_2 = ':;\"\'\\|/<>,.';
    let symbolsTreeChars = symbols_1.concat(symbols_2);
    testAllSubstrings(symbolsTreeChars);
}

function testSymbolsSlicedSubstring() {
    let symbols_1 = '`~!@#$%^&*()_-+={}[]';
    let symbols_2 = ':;\"\'\\|/<>,.';
    let symbolsTreeChars = symbols_1.concat(symbols_2);
    let symbolsSlicedChars = symbolsTreeChars.substring(1, 14);
    testAllSubstrings(symbolsSlicedChars);
}

function main(): int {
    /**
     * indexOf(string): int => StdCoreStringIndexOfString
     */
    let suite = new arktest.ArkTestsuite('StringIndexOfString');
    suite.addTest('testEmptyString', testEmptyString);
    suite.addTest('testTreeOneCharString', testTreeOneCharString);
    suite.addTest('testEscapedLinearString', testEscapedLinearString);
    suite.addTest('testEscapedLinearSubstring', testEscapedLinearSubstring);
    suite.addTest('testEscapedTreeSubstring', testEscapedTreeSubstring);
    suite.addTest('testEscapedMixedTreeSubstring', testEscapedMixedTreeSubstring);
    suite.addTest('testEscapedSlicedSubstring', testEscapedSlicedSubstring);
    suite.addTest('testShortLinearString', testShortLinearString);
    suite.addTest('testNonCompressedLinearString', testNonCompressedLinearString);
    suite.addTest('testNonCompressedLinearSubstring', testNonCompressedLinearSubstring);
    suite.addTest('testNonCompressedTreeString', testNonCompressedTreeString);
    suite.addTest('testNonCompressedTreeSubstring', testNonCompressedTreeSubstring);
    suite.addTest('testNonCompressedSlicedString', testNonCompressedSlicedString);
    suite.addTest('testNonCompressedSlicedSubstring', testNonCompressedSlicedSubstring);
    suite.addTest('testCompressedLinearString', testCompressedLinearString);
    suite.addTest('testCompressedLinearSubstring', testCompressedLinearSubstring);
    suite.addTest('testCompressedTreeString', testCompressedTreeString);
    suite.addTest('testCompressedTreeSubstring', testCompressedTreeSubstring);
    suite.addTest('testCompressedSlicedString', testCompressedSlicedString);
    suite.addTest('testCompressedSlicedSubstring', testCompressedSlicedSubstring);
    suite.addTest('testMixedCompressingLinearString', testMixedCompressingLinearString);
    suite.addTest('testMixedCompressingLinearSubstring', testMixedCompressingLinearSubstring);
    suite.addTest('testMixedCompressingTreeString', testMixedCompressingTreeString);
    suite.addTest('testMixedCompressingTreeSubstring', testMixedCompressingTreeSubstring);
    suite.addTest('testMixedCompressingSlicedString', testMixedCompressingSlicedString);
    suite.addTest('testMixedCompressingSlicedSubstring', testMixedCompressingSlicedSubstring);
    suite.addTest('testDupCharsLinearSubstring', testDupCharsLinearSubstring);
    suite.addTest('testDupCharsTreeSubstring', testDupCharsTreeSubstring);
    suite.addTest('testDupCharsSlicedSubstring', testDupCharsSlicedSubstring);
    suite.addTest('testDigitsLinearSubstring', testDigitsLinearSubstring);
    suite.addTest('testDigitsTreeSubstring', testDigitsTreeSubstring);
    suite.addTest('testDigitsSlicedSubstring', testDigitsSlicedSubstring);
    suite.addTest('testSymbolsLinearString', testSymbolsLinearString);
    suite.addTest('testSymbolsLinearSubstring', testSymbolsLinearSubstring);
    suite.addTest('testSymbolsTreeString', testSymbolsTreeString);
    suite.addTest('testSymbolsTreeSubstring', testSymbolsTreeSubstring);
    suite.addTest('testSymbolsSlicedString', testSymbolsSlicedString);
    suite.addTest('testSymbolsSlicedSubstring', testSymbolsSlicedSubstring);
    return suite.run();
}
