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

// Ideally we'd like to avoid call (and test) anything but String.lastIndexOf(char).
// In particular the array of chars to avoid calling String.charAt(pos).

const Empty: String = '';
const OneEngChar: String = 'x';                                                                //  8-bit
const OneRusChar: String = 'ж';                                                                // 16-bit
const EscChars: String = '\f\n\r\t\v';
const EscArray = new Array<char>(c'\f', c'\n', c'\r', c'\t', c'\v');

const RusChars: String = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя'; // 16-bit
const RusArray = new Array<char>(c'А', c'а', c'Б', c'б', c'В', c'в', c'Г', c'г', c'Д', c'д', c'Е', c'е', c'Ё', c'ё',
                               c'Ж', c'ж', c'З', c'з', c'И', c'и', c'Й', c'й', c'К', c'к', c'Л', c'л', c'М', c'м',
                               c'Н', c'н', c'О', c'о', c'П', c'п', c'Р', c'р', c'С', c'с', c'Т', c'т', c'У', c'у',
                               c'Ф', c'ф', c'Х', c'х', c'Ц', c'ц', c'Ч', c'ч', c'Ш', c'ш', c'Щ', c'щ', c'Ъ', c'ъ',
                               c'Ы', c'ы', c'Ь', c'ь', c'Э', c'э', c'Ю', c'ю', c'Я', c'я');
const EngChars: String = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz';               //  8-bit
const EngArray = new Array<char>(c'A', c'a', c'B', c'b', c'C', c'c', c'D', c'd', c'E', c'e', c'F', c'f', c'G', c'g',
                               c'H', c'h', c'I', c'i', c'J', c'j', c'K', c'k', c'L', c'l', c'M', c'm', c'N', c'n',
                               c'O', c'o', c'P', c'p', c'Q', c'q', c'R', c'r', c'S', c's', c'T', c't', c'U', c'u',
                               c'V', c'v', c'W', c'w', c'X', c'x', c'Y', c'y', c'Z', c'z');
const MixedChars: String = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнAaBbCcDdEeFfGgHhIiJjKkLl';           // 16-bit
const MixedArray = new Array<char>(c'А', c'а', c'Б', c'б', c'В', c'в', c'Г', c'г', c'Д', c'д', c'Е', c'е', c'Ё', c'ё',
                                 c'Ж', c'ж', c'З', c'з', c'И', c'и', c'Й', c'й', c'К', c'к', c'Л', c'л', c'М', c'м',
                                 c'Н', c'н', c'A', c'a', c'B', c'b', c'C', c'c', c'D', c'd', c'E', c'e', c'F', c'f',
                                 c'G', c'g', c'H', c'h', c'I', c'i', c'J', c'j', c'K', c'k', c'L', c'l');
const DupChars: String = 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA';               //  8-bit
const DupArray = Array.create<char>(DupChars.length, c'A');
const Digits: String = '0123456789';
const DigitArray = new Array<char>(c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9');
const Symbols: String = '`~!@#$%^&*()_-+={}[]:;\"\'\\|/<>,.';
const SymbolArray = new Array<char>(c'`', c'~', c'!', c'@', c'#', c'$', c'%', c'^', c'&', c'*', c'(', c')', c'_', c'-',
                                    c'+', c'=', c'{', c'}', c'[', c']', c':', c';', c'\"', c'\'', c'\\', c'|', c'/',
                                    c'<', c'>', c',', c'.');
const CharNotFound: int = -1;

function testAllChars(str: String, charArray: Array<char>, strict: boolean = true) {
    arktest.assertEQ(str.getLength(), charArray.length);
    for (let i = 0; i < str.getLength(); ++i) {
        arktest.assertEQ(str.indexOf(charArray[i]), strict ? i : 0);
    }

    for (let i = 0; i < str.getLength(); ++i) {
        for (let j = 0; j < str.getLength(); ++j) {
            let index: int = str.indexOf(charArray[i], j);
            if (strict) {
                arktest.assertEQ(index, i < j ? CharNotFound : i);
            } else {
                arktest.assertEQ(index, j);
            }
        }
    }
}

function testEmptyString() {
    // indexOf() without an explicitly specified position to start from (i.e. position is 0)
    arktest.assertEQ(Empty.indexOf(c'x'), CharNotFound);
    arktest.assertEQ(Empty.indexOf(c'x', Empty.length), CharNotFound);
}

function testOneEngCharString() {
    arktest.assertEQ(OneEngChar.indexOf(c'x'), 0);
    arktest.assertEQ(OneEngChar.indexOf(c'x', 0), 0);
    arktest.assertEQ(OneEngChar.indexOf(c'y'), CharNotFound);
    arktest.assertEQ(OneEngChar.indexOf(c'y', 0), CharNotFound);
    arktest.assertEQ(OneEngChar.indexOf(c'ж'), CharNotFound);
    arktest.assertEQ(OneEngChar.indexOf(c'ж', 0), CharNotFound);
}

function testOneRusCharString() {
    arktest.assertEQ(OneRusChar.indexOf(c'ж'), 0);
    arktest.assertEQ(OneRusChar.indexOf(c'ж', 0), 0);
    arktest.assertEQ(OneRusChar.indexOf(c'ы'), CharNotFound);
    arktest.assertEQ(OneRusChar.indexOf(c'ы', 0), CharNotFound);
    arktest.assertEQ(OneRusChar.indexOf(c's'), CharNotFound);
    arktest.assertEQ(OneRusChar.indexOf(c's', 0), CharNotFound);
}

function testShortLinearString() {
    let str: String = 'AcBbcdE';
    arktest.assertEQ(str.length, 8 - 1);
    arktest.assertEQ(str.indexOf(c'Ц'), CharNotFound);
    arktest.assertEQ(str.indexOf(c'D'), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A'), 0);
    arktest.assertEQ(str.indexOf(c'A', 0), 0);
    arktest.assertEQ(str.indexOf(c'A', -1), 0);
    arktest.assertEQ(str.indexOf(c'A', 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length - 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'c'), 1);
    arktest.assertEQ(str.indexOf(c'c', 1), 1);
    arktest.assertEQ(str.indexOf(c'c', 2), str.length - 'cdE'.length);
    arktest.assertEQ(str.indexOf(c'c', str.length - 'cdE'.length), str.length - 'cdE'.length);
    arktest.assertEQ(str.indexOf(c'c', str.length - 'cdE'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'd'), str.length - 'dE'.length);
    arktest.assertEQ(str.indexOf(c'E'), str.length - 1);
    arktest.assertEQ(str.indexOf(c'E', 0), str.length - 1);
    arktest.assertEQ(str.indexOf(c'E', -str.length), str.length - 1);
    arktest.assertEQ(str.indexOf(c'E', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf(c'E', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'E', str.length + 1), CharNotFound);

    str = 'AaBbcdEf';
    arktest.assertEQ(str.length, 8);
    arktest.assertEQ(str.indexOf(c'Ц'), CharNotFound);
    arktest.assertEQ(str.indexOf(c'D'), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A'), 0);
    arktest.assertEQ(str.indexOf(c'A', 0), 0);
    arktest.assertEQ(str.indexOf(c'A', -1), 0);
    arktest.assertEQ(str.indexOf(c'A', 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length - 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'a'), 1);
    arktest.assertEQ(str.indexOf(c'a', 1), 1);
    arktest.assertEQ(str.indexOf(c'a', 2), CharNotFound);
    arktest.assertEQ(str.indexOf(c'E'), str.length - 'dE'.length);
    arktest.assertEQ(str.indexOf(c'f'), str.length - 1);
    arktest.assertEQ(str.indexOf(c'f', 0), str.length - 1);
    arktest.assertEQ(str.indexOf(c'f', -str.length), str.length - 1);
    arktest.assertEQ(str.indexOf(c'f', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf(c'f', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'f', str.length + 1), CharNotFound);

    str = 'AфыBышл';
    arktest.assertEQ(str.length, 8 - 1);
    arktest.assertEQ(str.indexOf(c'z'), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A'), 0);
    arktest.assertEQ(str.indexOf(c'A', 0), 0);
    arktest.assertEQ(str.indexOf(c'A', -1), 0);
    arktest.assertEQ(str.indexOf(c'A', -str.length), 0);
    arktest.assertEQ(str.indexOf(c'A', 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length - 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'ф'), 1);
    arktest.assertEQ(str.indexOf(c'ф', 0), 1);
    arktest.assertEQ(str.indexOf(c'ф', 1), 1);
    arktest.assertEQ(str.indexOf(c'ф', 2), CharNotFound);
    arktest.assertEQ(str.indexOf(c'ы'), 2);
    arktest.assertEQ(str.indexOf(c'ы', 0), 2);
    arktest.assertEQ(str.indexOf(c'ы', -1), 2);
    arktest.assertEQ(str.indexOf(c'ы', -str.length), 2);
    arktest.assertEQ(str.indexOf(c'ы', str.length - 'ыВышл'.length), 2);
    arktest.assertEQ(str.indexOf(c'ы', str.length - 'ышл'.length), str.length - 'ышл'.length);
    arktest.assertEQ(str.indexOf(c'ы', str.length - 'ышл'.length - 1), str.length - 'ышл'.length);
    arktest.assertEQ(str.indexOf(c'ы', str.length - 'ышл'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'ы', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'ы', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'ш'), str.length - 'шл'.length);
    arktest.assertEQ(str.indexOf(c'ш', 0), str.length - 'шл'.length);
    arktest.assertEQ(str.indexOf(c'ш', -1), str.length - 'шл'.length);
    arktest.assertEQ(str.indexOf(c'ш', 1), str.length - 'шл'.length);
    arktest.assertEQ(str.indexOf(c'ш', str.length - 'шл'.length), str.length - 'шл'.length);
    arktest.assertEQ(str.indexOf(c'ш', str.length - 'шл'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'л'), str.length - 1);
    arktest.assertEQ(str.indexOf(c'л', 0), str.length - 1);
    arktest.assertEQ(str.indexOf(c'л', 3), str.length - 1);
    arktest.assertEQ(str.indexOf(c'л', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf(c'л', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'л', str.length + 1), CharNotFound);

    str = 'AфыBышлД';
    arktest.assertEQ(str.length, 8);
    arktest.assertEQ(str.indexOf(c'z'), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A'), 0);
    arktest.assertEQ(str.indexOf(c'A', 0), 0);
    arktest.assertEQ(str.indexOf(c'A', -1), 0);
    arktest.assertEQ(str.indexOf(c'A', -str.length), 0);
    arktest.assertEQ(str.indexOf(c'A', 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length - 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'ф'), 1);
    arktest.assertEQ(str.indexOf(c'ф', 0), 1);
    arktest.assertEQ(str.indexOf(c'ф', 1), 1);
    arktest.assertEQ(str.indexOf(c'ф', 2), CharNotFound);
    arktest.assertEQ(str.indexOf(c'ы'), 2);
    arktest.assertEQ(str.indexOf(c'ы', 0), 2);
    arktest.assertEQ(str.indexOf(c'ы', -1), 2);
    arktest.assertEQ(str.indexOf(c'ы', -str.length), 2);
    arktest.assertEQ(str.indexOf(c'ы', str.length - 'ыВышлД'.length - 1), 2);
    arktest.assertEQ(str.indexOf(c'ы', str.length - 'ышлД'.length), str.length - 'ышлД'.length);
    arktest.assertEQ(str.indexOf(c'ы', str.length - 'ышлД'.length - 1), str.length - 'ышлД'.length);
    arktest.assertEQ(str.indexOf(c'ы', str.length - 'ышлД'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'ы', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'ы', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'л'), str.length - 'лД'.length);
    arktest.assertEQ(str.indexOf(c'л', 0), str.length - 'лД'.length);
    arktest.assertEQ(str.indexOf(c'л', -1), str.length - 'лД'.length);
    arktest.assertEQ(str.indexOf(c'л', 1), str.length - 'лД'.length);
    arktest.assertEQ(str.indexOf(c'л', str.length - 'лД'.length), str.length - 'лД'.length);
    arktest.assertEQ(str.indexOf(c'л', str.length - 'лД'.length - 1), str.length - 'лД'.length);
    arktest.assertEQ(str.indexOf(c'л', str.length - 'лД'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'Д'), str.length - 1);
    arktest.assertEQ(str.indexOf(c'Д', 0), str.length - 1);
    arktest.assertEQ(str.indexOf(c'Д', 3), str.length - 1);
    arktest.assertEQ(str.indexOf(c'Д', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf(c'Д', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'Д', str.length + 1), CharNotFound);

    str = 'AфzBzшл';
    arktest.assertEQ(str.length, 8 - 1);
    arktest.assertEQ(str.indexOf(c'L'), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A'), 0);
    arktest.assertEQ(str.indexOf(c'A', 0), 0);
    arktest.assertEQ(str.indexOf(c'A', -1), 0);
    arktest.assertEQ(str.indexOf(c'A', -str.length), 0);
    arktest.assertEQ(str.indexOf(c'A', 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length - 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'ф'), 1);
    arktest.assertEQ(str.indexOf(c'ф', 0), 1);
    arktest.assertEQ(str.indexOf(c'ф', 1), 1);
    arktest.assertEQ(str.indexOf(c'ф', 2), CharNotFound);
    arktest.assertEQ(str.indexOf(c'z'), 2);
    arktest.assertEQ(str.indexOf(c'z', 0), 2);
    arktest.assertEQ(str.indexOf(c'z', -1), 2);
    arktest.assertEQ(str.indexOf(c'z', -str.length), 2);
    arktest.assertEQ(str.indexOf(c'z', str.length - 'zВzшл'.length - 1), 2);
    arktest.assertEQ(str.indexOf(c'z', str.length - 'zшл'.length), str.length - 'zшл'.length);
    arktest.assertEQ(str.indexOf(c'z', str.length - 'zшл'.length - 1), str.length - 'zшл'.length);
    arktest.assertEQ(str.indexOf(c'z', str.length - 'zшл'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'z', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'z', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'ш'), str.length - 'шл'.length);
    arktest.assertEQ(str.indexOf(c'ш', 0), str.length - 'шл'.length);
    arktest.assertEQ(str.indexOf(c'ш', -1), str.length - 'шл'.length);
    arktest.assertEQ(str.indexOf(c'ш', 1), str.length - 'шл'.length);
    arktest.assertEQ(str.indexOf(c'ш', str.length - 'шл'.length), str.length - 'шл'.length);
    arktest.assertEQ(str.indexOf(c'ш', str.length - 'шл'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'л'), str.length - 1);
    arktest.assertEQ(str.indexOf(c'л', 0), str.length - 1);
    arktest.assertEQ(str.indexOf(c'л', 3), str.length - 1);
    arktest.assertEQ(str.indexOf(c'л', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf(c'л', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'л', str.length + 1), CharNotFound);

    str = 'AфzBzшлL';
    arktest.assertEQ(str.length, 8);
    arktest.assertEQ(str.indexOf(c'S'), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A'), 0);
    arktest.assertEQ(str.indexOf(c'A', 0), 0);
    arktest.assertEQ(str.indexOf(c'A', -1), 0);
    arktest.assertEQ(str.indexOf(c'A', -str.length), 0);
    arktest.assertEQ(str.indexOf(c'A', 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length - 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'ф'), 1);
    arktest.assertEQ(str.indexOf(c'ф', 0), 1);
    arktest.assertEQ(str.indexOf(c'ф', 1), 1);
    arktest.assertEQ(str.indexOf(c'ф', 2), CharNotFound);
    arktest.assertEQ(str.indexOf(c'z'), 2);
    arktest.assertEQ(str.indexOf(c'z', 0), 2);
    arktest.assertEQ(str.indexOf(c'z', -1), 2);
    arktest.assertEQ(str.indexOf(c'z', -str.length), 2);
    arktest.assertEQ(str.indexOf(c'z', str.length - 'zВzшлL'.length), 2);
    arktest.assertEQ(str.indexOf(c'z', str.length - 'zшлL'.length), str.length - 'zшлL'.length);
    arktest.assertEQ(str.indexOf(c'z', str.length - 'zшлL'.length - 1), str.length - 'zшлL'.length);
    arktest.assertEQ(str.indexOf(c'z', str.length - 'zшлL'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'z', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'z', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'л'), str.length - 'лL'.length);
    arktest.assertEQ(str.indexOf(c'л', 0), str.length - 'лL'.length);
    arktest.assertEQ(str.indexOf(c'л', -1), str.length - 'лL'.length);
    arktest.assertEQ(str.indexOf(c'л', 1), str.length - 'лL'.length);
    arktest.assertEQ(str.indexOf(c'л', str.length - 'лL'.length - 1), str.length - 'лL'.length);
    arktest.assertEQ(str.indexOf(c'л', str.length - 'лL'.length), str.length - 'лL'.length);
    arktest.assertEQ(str.indexOf(c'л', str.length - 'лL'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'L'), str.length - 1);
    arktest.assertEQ(str.indexOf(c'L', 0), str.length - 1);
    arktest.assertEQ(str.indexOf(c'L', 3), str.length - 1);
    arktest.assertEQ(str.indexOf(c'L', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf(c'L', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'L', str.length + 1), CharNotFound);
}

function testMiddleTreeString() {
    let str = ('super '.concat('--')).concat('alphA'.concat('!'));
    arktest.assertEQ(str.length, 14); // the minimum length of a tree string
    arktest.assertEQ(str.indexOf(c'C'), CharNotFound);
    arktest.assertEQ(str.indexOf(c'Ц'), CharNotFound);
    arktest.assertEQ(str.indexOf(c's'), 0);
    arktest.assertEQ(str.indexOf(c's', 0), 0);
    arktest.assertEQ(str.indexOf(c's', -1), 0);
    arktest.assertEQ(str.indexOf(c's', -str.length), 0);
    arktest.assertEQ(str.indexOf(c's', 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'-'), str.length - '--alphA!'.length);
    arktest.assertEQ(str.indexOf(c'-', 1), str.length - '--alphA!'.length);
    arktest.assertEQ(str.indexOf(c'-', 0), str.length - '--alphA!'.length);
    arktest.assertEQ(str.indexOf(c'-', -1), str.length - '--alphA!'.length);
    arktest.assertEQ(str.indexOf(c'-', str.length - '--alphA!'.length), str.length - '--alphA!'.length);
    arktest.assertEQ(str.indexOf(c'-', str.length - '-alphA!'.length), str.length - '-alphA!'.length);
    arktest.assertEQ(str.indexOf(c'-', str.length - '-alphA!'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'-', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'-', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A'), str.length - 'A!'.length);
    arktest.assertEQ(str.indexOf(c'A', 0), str.length - 'A!'.length);
    arktest.assertEQ(str.indexOf(c'A', -1), str.length - 'A!'.length);
    arktest.assertEQ(str.indexOf(c'A', str.length - 'A!'.length - 1), str.length - 'A!'.length);
    arktest.assertEQ(str.indexOf(c'A', str.length - 'A!'.length), str.length - 'A!'.length);
    arktest.assertEQ(str.indexOf(c'A', str.length - 'A!'.length  + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', -str.length), str.length - 'A!'.length);
    arktest.assertEQ(str.indexOf(c'A', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'A', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'!'), str.length - 1);
    arktest.assertEQ(str.indexOf(c'!', 0), str.length - 1);
    arktest.assertEQ(str.indexOf(c'!', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf(c'!', str.length), CharNotFound);
}

function testMiddleLinearString() {
    let str = 'ZyxWQqPpRrSsaBb';
    arktest.assertEQ(str.length, 16 - 1);
    arktest.assertEQ(str.indexOf(c'Z'), 0);
    arktest.assertEQ(str.indexOf(c'Z', -1), 0);
    arktest.assertEQ(str.indexOf(c'Z', -str.length), 0);
    arktest.assertEQ(str.indexOf(c'Z', 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'Z', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'Z', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'!'), CharNotFound);
    arktest.assertEQ(str.indexOf(c'Ы'), CharNotFound);
    arktest.assertEQ(str.indexOf(c'B'), str.length - 'Bb'.length);
    arktest.assertEQ(str.indexOf(c'B', -1), str.length - 'Bb'.length);
    arktest.assertEQ(str.indexOf(c'B', str.length - 'Bb'.length - 1), str.length - 'Bb'.length);
    arktest.assertEQ(str.indexOf(c'B', str.length - 'Bb'.length), str.length - 'Bb'.length);
    arktest.assertEQ(str.indexOf(c'B', str.length - 'Bb'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'B', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'B', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf(c'b'), str.length - 1);
    arktest.assertEQ(str.indexOf(c'b', -1), str.length - 1);
    arktest.assertEQ(str.indexOf(c'b', str.length - 2), str.length - 1);
    arktest.assertEQ(str.indexOf(c'b', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf(c'b', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf(c'b', str.length + 1), CharNotFound);

    str = 'OpqrstuvwxzAaBab';
    arktest.assertEQ(str.length, 16);
    arktest.assertEQ(str.indexOf('O'), 0);
    arktest.assertEQ(str.indexOf('O', -1), 0);
    arktest.assertEQ(str.indexOf('O', -str.length), 0);
    arktest.assertEQ(str.indexOf('O', 1), CharNotFound);
    arktest.assertEQ(str.indexOf('O', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('O', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('Ы'), CharNotFound);
    arktest.assertEQ(str.indexOf('!'), CharNotFound);
    arktest.assertEQ(str.indexOf('a'), str.length - 'aBab'.length);
    arktest.assertEQ(str.indexOf('a', 0), str.length - 'aBab'.length);
    arktest.assertEQ(str.indexOf('a', 1), str.length - 'aBab'.length);
    arktest.assertEQ(str.indexOf('a', -1), str.length - 'aBab'.length);
    arktest.assertEQ(str.indexOf('a', str.length - 1), CharNotFound);
    arktest.assertEQ(str.indexOf('a', str.length - 'ab'.length), str.length - 'ab'.length);
    arktest.assertEQ(str.indexOf('a', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('a', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('b'), str.length - 1);
    arktest.assertEQ(str.indexOf('b', 0), str.length - 1);
    arktest.assertEQ(str.indexOf('b', 1), str.length - 1);
    arktest.assertEQ(str.indexOf('b', -1), str.length - 1);
    arktest.assertEQ(str.indexOf('b', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf('b', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('b', str.length + 1), CharNotFound);

    str = 'штзрЬоЯЯЙЫхфыВы';
    arktest.assertEQ(str.length, 16 - 1);
    arktest.assertEQ(str.indexOf(c'z'), CharNotFound);
    arktest.assertEQ(str.indexOf('ш'), 0);
    arktest.assertEQ(str.indexOf('ш', -1), 0);
    arktest.assertEQ(str.indexOf('ш', -str.length), 0);
    arktest.assertEQ(str.indexOf('ш', 1), CharNotFound);
    arktest.assertEQ(str.indexOf('ш', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('ш', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('Ц'), CharNotFound);
    arktest.assertEQ(str.indexOf('ф'), str.length - 'фыВы'.length);
    arktest.assertEQ(str.indexOf('В'), str.length - 'Вы'.length);
    arktest.assertEQ(str.indexOf('В', str.length - 'Вы'.length), str.length - 'Вы'.length);
    arktest.assertEQ(str.indexOf('В', str.length - 'Вы'.length - 1), str.length - 'Вы'.length);
    arktest.assertEQ(str.indexOf('В', str.length - 'Вы'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('ы'), str.length - 'ыВы'.length);
    arktest.assertEQ(str.indexOf('ы', -1), str.length - 'ыВы'.length);
    arktest.assertEQ(str.indexOf('ы', str.length - 'фыВы'.length), str.length - 'ыВы'.length);
    arktest.assertEQ(str.indexOf('ы', str.length - 'ыВы'.length), str.length - 'ыВы'.length);
    arktest.assertEQ(str.indexOf('ы', str.length - 'Вы'.length), str.length - 1);
    arktest.assertEQ(str.indexOf('ы', str.length - 'ы'.length), str.length - 1);
    arktest.assertEQ(str.indexOf('ы', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('ы', str.length + 1), CharNotFound);

    str = 'штзрЬоЯЯЙЫхAфzWz';
    arktest.assertEQ(str.length, 16);
    arktest.assertEQ(str.indexOf('ш'), 0);
    arktest.assertEQ(str.indexOf('ш', -1), 0);
    arktest.assertEQ(str.indexOf('ш', -str.length), 0);
    arktest.assertEQ(str.indexOf('ш', 1), CharNotFound);
    arktest.assertEQ(str.indexOf('ш', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('ш', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('Q'), CharNotFound);
    arktest.assertEQ(str.indexOf('Ю'), CharNotFound);
    arktest.assertEQ(str.indexOf('W'), str.length - 'Wz'.length);
    arktest.assertEQ(str.indexOf('W', str.length - 'Wz'.length), str.length - 'Wz'.length);
    arktest.assertEQ(str.indexOf('W', str.length - 'Wz'.length - 1), str.length - 'Wz'.length);
    arktest.assertEQ(str.indexOf('W', str.length - 'Wz'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('z'), str.length - 'zWz'.length);
    arktest.assertEQ(str.indexOf('z', -1), str.length - 'zWz'.length);
    arktest.assertEQ(str.indexOf('z', str.length - 'фzWz'.length), str.length - 'zWz'.length);
    arktest.assertEQ(str.indexOf('z', str.length - 'zWz'.length), str.length - 'zWz'.length);
    arktest.assertEQ(str.indexOf('z', str.length - 'Wz'.length), str.length - 1);
    arktest.assertEQ(str.indexOf('z', str.length - 'z'.length), str.length - 1);
    arktest.assertEQ(str.indexOf('z', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('z', str.length + 1), CharNotFound);
}

function testEscapedLinearChars() {
    testAllChars(EscChars, EscArray);
}

function testEscapedTreeChars() {
    let mixedTreeChars = 'Alphest' + (EscChars.concat('b.?!'));
    let mixedTreeArray = new Array<char>(c'A', c'l', c'p', c'h', c'e', c's', c't').concat(EscArray)
        .concat(new Array<char>(c'b', c'.', c'?', c'!'));
    arktest.assertLT(14, mixedTreeChars.length)  // the minimum length of a tree string
    testAllChars(mixedTreeChars, mixedTreeArray);
}

function testEscapedTreeDuplicates() {
    let escapedTreeChars = (EscChars.concat(EscChars)).concat((EscChars.concat(EscChars)));
    arktest.assertLT(14, escapedTreeChars.length)  // the minimum length of a tree string
    arktest.assertEQ(escapedTreeChars.indexOf(c'\n'), 1);
    arktest.assertEQ(escapedTreeChars.indexOf(c'\n', 0), 1);
    arktest.assertEQ(escapedTreeChars.indexOf(c'\n', -1), 1);
    arktest.assertEQ(escapedTreeChars.indexOf(c'\n', 1), 1);
    arktest.assertEQ(escapedTreeChars.indexOf(c'\n', 2), EscChars.length + 1);
    arktest.assertEQ(escapedTreeChars.indexOf(c'\n', EscChars.length), EscChars.length + 1);
    arktest.assertEQ(escapedTreeChars.indexOf(c'\n', EscChars.length + 1), EscChars.length + 1);
    arktest.assertEQ(escapedTreeChars.indexOf(c'\n', EscChars.length + 2), 2 * EscChars.length + 1);
}

function testEscapedMixedTreeChars() {
    let mixedTreeChars = EscChars.concat(RusChars);
    let mixedTreeArray = EscArray.concat(RusArray);
    testAllChars(mixedTreeChars, mixedTreeArray);
}

function testEscapedSlicedChars() {
    let escSlicedChars = EscChars.substring(1, EscChars.length - 2);
    testAllChars(escSlicedChars, EscArray.slice(1, EscArray.length - 2));
}

function testNonCompressedLinearString() {
    arktest.assertEQ(RusChars.length, 66);
    arktest.assertEQ(RusChars.indexOf(c'Ѫ'), CharNotFound);
    arktest.assertEQ(RusChars.indexOf(c'Ѫ', 0), CharNotFound);
    arktest.assertEQ(RusChars.indexOf(c'Ѫ', -1), CharNotFound);
    arktest.assertEQ(RusChars.indexOf(c'Ѫ', 1), CharNotFound);

    // Check the case when 'position' is negative
    arktest.assertEQ(RusChars.indexOf(RusArray[0], -1), 0);
    // Check the case when 'position' is out of range
    arktest.assertEQ(RusChars.indexOf(RusArray[0], RusChars.getLength() * 2), CharNotFound);
}

function testNonCompressedLinearChars() {
    arktest.assertEQ(RusChars.length, 66);
    testAllChars(RusChars, RusArray);
}

function testNonCompressedTreeChars() {
    let rusChars_1 = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let rusChars_2 = 'ОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя';
    let rusTreeChars = rusChars_1.concat(rusChars_2);
    arktest.assertTrue(rusTreeChars === RusChars);
    arktest.assertTrue(RusChars === rusTreeChars);
    testAllChars(rusTreeChars, RusArray);

    let rusTreeChars2 = rusTreeChars.concat('!');
    testAllChars(rusTreeChars2, RusArray.concat(Array.create<char>(1, c'!')));
}

function testNonCompressedSlicedChars() {
    let rusChars_1 = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let rusChars_2 = 'ОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя';
    let rusTreeChars = rusChars_1.concat(rusChars_2);
    let rusSlicedChars = rusTreeChars.substring(3, 48);
    testAllChars(rusSlicedChars, RusArray.slice(3, 48));

    let rusTreeChars2 = rusChars_1.concat(rusChars_2).concat('零');
    let rusSlicedChars2 = rusTreeChars2.substring(3, 49);
    arktest.assertEQ(rusSlicedChars2.length, 46);
    testAllChars(rusSlicedChars2, RusArray.concat(Array.create<char>(1, c'零')).slice(3, 49));
}

function testCompressedLinearString() {
    arktest.assertEQ(EngChars.indexOf(c'æ'), CharNotFound);
    arktest.assertEQ(EngChars.indexOf(c'æ', 0), CharNotFound);
    arktest.assertEQ(EngChars.indexOf(c'æ', -1), CharNotFound);
    arktest.assertEQ(EngChars.indexOf(c'æ', 1), CharNotFound);

    // Check the case when 'position' is negative
    arktest.assertEQ(EngChars.indexOf(EngArray[0], -1), 0);
    // Check the case when 'position' is out of range
    arktest.assertEQ(EngChars.indexOf(EngArray[0], EngChars.getLength() * 2), CharNotFound);
}

function testCompressedLinearChars() {
    arktest.assertEQ(EngChars.length, 52);
    testAllChars(EngChars, EngArray);
}

function testCompressedTreeChars() {
    let engChars_1 = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNn';
    let engChars_2 = 'OoPpQqRrSsTtUuVvWwXxYyZz';
    let engTreeChars = engChars_1.concat(engChars_2);
    arktest.assertTrue(engTreeChars === EngChars);
    arktest.assertTrue(EngChars === engTreeChars);
    testAllChars(engTreeChars, EngArray);
    let engTreeChars2 = engTreeChars.concat('!');
    testAllChars(engTreeChars2, EngArray.concat(Array.create<char>(1, c'!')));
}

function testCompressedSlicedChars() {
    let engChars_1 = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNn';
    let engChars_2 = 'OoPpQqRrSsTtUuVvWwXxYyZz';
    let engTreeChars = engChars_1.concat(engChars_2);
    let engSlicedChars = engTreeChars.substring(3, 43);
    testAllChars(engSlicedChars, EngArray.slice(3, 43));

    let engTreeChars2 = engChars_1.concat(engChars_2).concat('!');
    let engSlicedChars2 = engTreeChars2.substring(3, 44);
    testAllChars(engSlicedChars2, EngArray.concat(Array.create<char>(1, c'!')).slice(3, 44));
}

function testMixedCompressingLinearChars() {
    testAllChars(MixedChars, MixedArray);
}

function testMixedCompressingTreeChars() {
    let rusChars = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let engChars = 'AaBbCcDdEeFfGgHhIiJjKkLl';
    let mixedTreeChars = rusChars.concat(engChars);
    arktest.assertTrue(mixedTreeChars === MixedChars);
    arktest.assertTrue(MixedChars === mixedTreeChars);
    testAllChars(mixedTreeChars, MixedArray);

    let mixedTreeChars2 = mixedTreeChars.concat('!');
    testAllChars(mixedTreeChars2, MixedArray.concat(Array.create<char>(1, c'!')));
}

function testMixedCompressingSlicedChars() {
    let rusChars = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let engChars = 'AaBbCcDdEeFfGgHhIiJjKkLl';
    arktest.assertLT(15, engChars.length);
    let mixedTreeChars = rusChars.concat(engChars);
    let mixedSlicedChars = mixedTreeChars.substring(3, rusChars.length + 15);
    arktest.assertEQ(mixedSlicedChars.length, rusChars.length + 15 - 3);
    testAllChars(mixedSlicedChars, MixedArray.slice(3, rusChars.length + 15));

    let mixedSlicedChars2 = (rusChars.concat(engChars).concat('!!!')).substring(1, rusChars.length
        + engChars.length + 1);
    let mixedSlicedArray2 = MixedArray.concat(Array.create<char>(3, c'!')).slice(1, rusChars.length
        + engChars.length + 1);
    arktest.assertLT(mixedSlicedChars.length, mixedSlicedChars2.length);
    testAllChars(mixedSlicedChars2, mixedSlicedArray2);
}

function testDupCharsLinearChars() {
    testAllChars(DupChars, DupArray, false);
}

function testDupCharsTreeChars() {
    let mixedTreeChars = DupChars.concat(DupChars);
    testAllChars(mixedTreeChars, DupArray.concat(DupArray), false);
}

function testDupCharsSlicedChars() {
    let dupSlicedChars = DupChars.substring(1, 26);
    arktest.assertEQ(dupSlicedChars.length, 25);
    testAllChars(dupSlicedChars, DupArray.slice(1, 26), false);
}

function testDigitsLinearChars() {
    testAllChars(Digits, DigitArray);
}

function testDigitsTreeChars() {
    let mixedTreeChars = '-+*;' + (Digits.concat('/?^'));
    let mixedTreeArray = new Array<char>(c'-', c'+', c'*', c';', c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8',
                                         c'9', c'/', c'?', c'^');
    arktest.assertLT(14, mixedTreeChars.length)  // the minimum length of a tree string
    testAllChars(mixedTreeChars, mixedTreeArray);
}

function testDigitsTreeDuplicates() {
    let digitTreeChars = Digits.concat(Digits);
    arktest.assertEQ(digitTreeChars.indexOf(c'0'), 0);
    arktest.assertEQ(digitTreeChars.indexOf(c'0', -1), 0);
    arktest.assertEQ(digitTreeChars.indexOf(c'0', 0), 0);
    arktest.assertEQ(digitTreeChars.indexOf(c'0', 1), Digits.length);
    arktest.assertEQ(digitTreeChars.indexOf(c'0', 2), Digits.length);
    arktest.assertEQ(digitTreeChars.indexOf(c'0', Digits.length - 1), Digits.length);
    arktest.assertEQ(digitTreeChars.indexOf(c'0', Digits.length), Digits.length);
    arktest.assertEQ(digitTreeChars.indexOf(c'0', Digits.length + 1), CharNotFound);
}

function testDigitsSlicedChars() {
    let digitsSlicedChars = Digits.substring(1, 8);
    arktest.assertEQ(digitsSlicedChars.length, 7);
    testAllChars(digitsSlicedChars, DigitArray.slice(1, 8));
}

function testSymbolsLinearChars() {
    testAllChars(Symbols, SymbolArray);
}

function testSymbolsTreeChars() {
    let symbols_1 = '`~!@#$%^&*()_-+={}[]';
    let symbols_2 = ':;\"\'\\|/<>,.';
    let symbolsTreeChars = symbols_1.concat(symbols_2);
    arktest.assertLT(14, symbolsTreeChars.length)  // the minimum length of a tree string
    testAllChars(symbolsTreeChars, SymbolArray);

    let symbolsTreeCharsLong = symbolsTreeChars.concat('X');
    let symbolsTreeLongArray = SymbolArray.concat(Array.create<char>(1, c'X'));
    testAllChars(symbolsTreeCharsLong, symbolsTreeLongArray);
}

function testSymbolsSlicedChars() {
    let symbols_1 = '`~!@#$%^&*()_-+={}[]';
    let symbols_2 = ':;\"\'\\|/<>,.';
    let symbolsTreeChars = symbols_1.concat(symbols_2);
    let symbolsSlicedChars = symbolsTreeChars.substring(1, 14);
    testAllChars(symbolsSlicedChars, SymbolArray.slice(1, 14));

    let symbolsTreeCharsLong = symbolsTreeChars.concat('XXX');
    let symbolsSlicedCharsLong = symbolsTreeCharsLong.substring(1, symbolsTreeCharsLong.length - 2);
    let symbolsSlicedLongArray = SymbolArray.concat(Array.create<char>(3, c'X')).slice(1, symbolsTreeCharsLong.length - 2);
    testAllChars(symbolsSlicedCharsLong, symbolsSlicedLongArray);
}

function main(): int {
    let suite = new arktest.ArkTestsuite('StringIndexOf');
    suite.addTest('testEmptyString', testEmptyString);
    suite.addTest('testOneEngCharString', testOneEngCharString);
    suite.addTest('testOneRusCharString', testOneRusCharString);
    suite.addTest('testEscapedLinearChars', testEscapedLinearChars);
    suite.addTest('testEscapedTreeChars', testEscapedTreeChars);
    suite.addTest('testEscapedTreeDuplicates', testEscapedTreeDuplicates);
    suite.addTest('testEscapedMixedTreeChars', testEscapedMixedTreeChars);
    suite.addTest('testEscapedSlicedChars', testEscapedSlicedChars);
    suite.addTest('testShortLinearString', testShortLinearString);
    suite.addTest('testMiddleLinearString', testMiddleLinearString);
    suite.addTest('testMiddleTreeString', testMiddleTreeString);
    suite.addTest('testNonCompressedLinearString', testNonCompressedLinearString);
    suite.addTest('testNonCompressedLinearChars', testNonCompressedLinearChars);
    suite.addTest('testNonCompressedTreeChars', testNonCompressedTreeChars);
    suite.addTest('testNonCompressedSlicedChars', testNonCompressedSlicedChars);
    suite.addTest('testCompressedLinearString', testCompressedLinearString);
    suite.addTest('testCompressedLinearChars', testCompressedLinearChars);
    suite.addTest('testCompressedTreeChars', testCompressedTreeChars);
    suite.addTest('testCompressedSlicedChars', testCompressedSlicedChars);
    suite.addTest('testMixedCompressingLinearChars', testMixedCompressingLinearChars);
    suite.addTest('testMixedCompressingTreeChars', testMixedCompressingTreeChars);
    suite.addTest('testMixedCompressingSlicedChars', testMixedCompressingSlicedChars);
    suite.addTest('testDupCharsLinearChars', testDupCharsLinearChars);
    suite.addTest('testDupCharsTreeChars', testDupCharsTreeChars);
    suite.addTest('testDupCharsSlicedChars', testDupCharsSlicedChars);
    suite.addTest('testDigitsLinearChars', testDigitsLinearChars);
    suite.addTest('testDigitsTreeChars', testDigitsTreeChars);
    suite.addTest('testDigitsTreeDuplicates', testDigitsTreeDuplicates);
    suite.addTest('testDigitsSlicedChars', testDigitsSlicedChars);
    suite.addTest('testSymbolsLinearChars', testSymbolsLinearChars);
    suite.addTest('testSymbolsTreeChars', testSymbolsTreeChars);
    suite.addTest('testSymbolsSlicedChars', testSymbolsSlicedChars);
    return suite.run();
}
