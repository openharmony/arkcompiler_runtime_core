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
    for (let i = 0; i < str.getLength(); ++i) {
        arktest.assertEQ(str.lastIndexOf(str.substring(i, str.length)), i);
    }
    for (let i = 1; i < str.getLength(); ++i) {
        if (strict) {
            arktest.assertEQ(str.lastIndexOf(str.substring(0, i), i + 1), 0);
        } else {
            arktest.assertEQ(str.lastIndexOf(str.substring(0, i)), str.length - i);
        }
    }
}

function testEmptyString() {
    arktest.assertEQ(Empty.lastIndexOf(''.concat('x')), CharNotFound);
    arktest.assertEQ(Empty.lastIndexOf(''.concat('x'), Empty.length), CharNotFound);
    arktest.assertEQ(Empty.lastIndexOf(''.concat('')), 0);
    arktest.assertEQ(Empty.lastIndexOf(''.concat(''), Empty.length + 1), 0);
    arktest.assertEQ(Empty.lastIndexOf(''.concat(''), Empty.length), 0);
    arktest.assertEQ(Empty.lastIndexOf(''.concat(''), 0), 0);
    arktest.assertEQ(Empty.lastIndexOf(''), 0);
    arktest.assertEQ(Empty.lastIndexOf('', 0), 0);
    arktest.assertEQ(Empty.lastIndexOf('x'), CharNotFound);
    arktest.assertEQ(Empty.lastIndexOf('x', Empty.length), CharNotFound);
}

function testTreeOneCharString() {
    arktest.assertEQ(OneChar.lastIndexOf(''.concat('x')), 0);
    arktest.assertEQ(OneChar.lastIndexOf(''.concat('x'), OneChar.length), 0);
    arktest.assertEQ(OneChar.lastIndexOf(''.concat('x'), 0), 0);
    arktest.assertEQ(OneChar.lastIndexOf('x'), 0);
    arktest.assertEQ(OneChar.lastIndexOf('x', OneChar.length), 0);
    arktest.assertEQ(OneChar.lastIndexOf('x', OneChar.length + 1), 0);
    arktest.assertEQ(OneChar.lastIndexOf('x', 0), 0);
    arktest.assertEQ(OneChar.lastIndexOf('xx', OneChar.length), CharNotFound);
    arktest.assertEQ(OneChar.lastIndexOf('xi', OneChar.length), CharNotFound);
    arktest.assertEQ(OneChar.lastIndexOf('xi', OneChar.length + 1), CharNotFound);
    arktest.assertEQ(OneChar.lastIndexOf(''.concat('l'), OneChar.length), CharNotFound);
    arktest.assertEQ(OneChar.lastIndexOf('l', OneChar.length), CharNotFound);
    arktest.assertEQ(OneChar.lastIndexOf('l', OneChar.length + 1), CharNotFound);
    arktest.assertEQ(OneChar.lastIndexOf('hi', OneChar.length), CharNotFound);
    arktest.assertEQ(OneChar.lastIndexOf('h'.concat('k'), OneChar.length), CharNotFound);
}

function testShortLinearString() {
    let str = 'A';
    arktest.assertEQ(str.lastIndexOf('Aa'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('C'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Ц'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('A'), 0);
    arktest.assertEQ(str.lastIndexOf('A', 0), 0);
    arktest.assertEQ(str.lastIndexOf('A', -1), 0);
    arktest.assertEQ(str.lastIndexOf('A', -str.length), 0);
    arktest.assertEQ(str.lastIndexOf('A', str.length), 0);
    arktest.assertEQ(str.lastIndexOf('A', str.length + 1), 0);

    str = 'AaBb';
    arktest.assertEQ(str.lastIndexOf('AaBb'), 0);
    arktest.assertEQ(str.lastIndexOf('AaBb', 0), 0);
    arktest.assertEQ(str.lastIndexOf('AaBb', -1), 0);
    arktest.assertEQ(str.lastIndexOf('AaBb', -str.length), 0);
    arktest.assertEQ(str.lastIndexOf('AaBb', str.length), 0);
    arktest.assertEQ(str.lastIndexOf('AaBb', str.length + 1), 0);
    arktest.assertEQ(str.lastIndexOf('ABBb'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('aBb'), 1);
    arktest.assertEQ(str.lastIndexOf('aBb', str.length), 1);
    arktest.assertEQ(str.lastIndexOf('aBb', str.length + 1), 1);
    arktest.assertEQ(str.lastIndexOf('aBb', str.length - 1), 1);
    arktest.assertEQ(str.lastIndexOf('aBb', 1), 1);
    arktest.assertEQ(str.lastIndexOf('aBb', 2), 1);
    arktest.assertEQ(str.lastIndexOf('aBb', 3), 1);
    arktest.assertEQ(str.lastIndexOf('aBb', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('aBz'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('aXb'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('b'), 3);
    arktest.assertEQ(str.lastIndexOf('b', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('b', 4), 3);
    arktest.assertEQ(str.lastIndexOf('b', 3), 3);
    arktest.assertEQ(str.lastIndexOf('b', 2), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('ц'), CharNotFound);

    str = 'AaBab';
    arktest.assertEQ(str.lastIndexOf('AaBabx'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('A'), 0);
    arktest.assertEQ(str.lastIndexOf('A', -1), 0);
    arktest.assertEQ(str.lastIndexOf('A', -str.length), 0);
    arktest.assertEQ(str.lastIndexOf('A', 0), 0);
    arktest.assertEQ(str.lastIndexOf('A', 1), 0);
    arktest.assertEQ(str.lastIndexOf('A', str.length), 0);
    arktest.assertEQ(str.lastIndexOf('A', str.length + 1), 0);
    arktest.assertEQ(str.lastIndexOf('AaBab'), 0);
    arktest.assertEQ(str.lastIndexOf('AaB'), 0);
    arktest.assertEQ(str.lastIndexOf('AaB', -1), 0);
    arktest.assertEQ(str.lastIndexOf('AaB', -str.length), 0);
    arktest.assertEQ(str.lastIndexOf('AaB', 1), 0);
    arktest.assertEQ(str.lastIndexOf('AaB', str.length), 0);
    arktest.assertEQ(str.lastIndexOf('AaB', str.length - 1), 0);
    arktest.assertEQ(str.lastIndexOf('AaB', str.length + 1), 0);
    arktest.assertEQ(str.lastIndexOf('AaBab', -1), 0);
    arktest.assertEQ(str.lastIndexOf('AaBaz'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('ABBab'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('aBab'), 1);
    arktest.assertEQ(str.lastIndexOf('aBab', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('aBab', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('aBab', -str.length), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('aBab', 1), 1);
    arktest.assertEQ(str.lastIndexOf('aBab', 2), 1);
    arktest.assertEQ(str.lastIndexOf('aXab'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Ba'), str.length - 'Bab'.length);
    arktest.assertEQ(str.lastIndexOf('Ba', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Bb'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('a'), str.length - 2);
    arktest.assertEQ(str.lastIndexOf('a', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('a', 1), 1);
    arktest.assertEQ(str.lastIndexOf('a', 2), 1);
    arktest.assertEQ(str.lastIndexOf('a', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('a', -str.length), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('a', str.length - 1), str.length - 2);
    arktest.assertEQ(str.lastIndexOf('a', str.length - 2), str.length - 2);
    arktest.assertEQ(str.lastIndexOf('a', str.length), str.length - 2);
    arktest.assertEQ(str.lastIndexOf('a', str.length + 1), str.length - 2);
    arktest.assertEQ(str.lastIndexOf('b'), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('b', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('b', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('b', str.length - 1), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('b', str.length - 2), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('b', str.length + 1), str.length - 1);

    str = 'AbbA';
    arktest.assertEQ(str.lastIndexOf('A'), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('A', str.length), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('A', str.length + 1), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('A', 0), 0);
    arktest.assertEQ(str.lastIndexOf('A', -1), 0);
    arktest.assertEQ(str.lastIndexOf('A', str.length - 'bA'.length), 0);

    str = 'Ф';
    arktest.assertEQ(str.lastIndexOf('Фв'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('б'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Ф'), 0);
    arktest.assertEQ(str.lastIndexOf('S'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Ф', 0), 0);
    arktest.assertEQ(str.lastIndexOf('Ф', -1), 0);
    arktest.assertEQ(str.lastIndexOf('Ф', -str.length), 0);
    arktest.assertEQ(str.lastIndexOf('Ф', str.length), 0);
    arktest.assertEQ(str.lastIndexOf('Ф', str.length + 1), 0);

    str = 'AфыBы';
    arktest.assertEQ(str.lastIndexOf('AфыBы'), 0);
    arktest.assertEQ(str.lastIndexOf('AЙыBb'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AфыB'), 0);
    arktest.assertEQ(str.lastIndexOf('AфыB', -1), 0);
    arktest.assertEQ(str.lastIndexOf('AфыB', str.length), 0);
    arktest.assertEQ(str.lastIndexOf('AфыB', str.length + 1), 0);
    arktest.assertEQ(str.lastIndexOf('AфыB', 2), 0);
    arktest.assertEQ(str.lastIndexOf('AфыB', 1), 0);
    arktest.assertEQ(str.lastIndexOf('AфыB', 0), 0);
    arktest.assertEQ(str.lastIndexOf('фыB'), 1);
    arktest.assertEQ(str.lastIndexOf('фыB', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('фыB', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('фыB', 1), 1);
    arktest.assertEQ(str.lastIndexOf('фыB', 2), 1);
    arktest.assertEQ(str.lastIndexOf('ф'), 1);
    arktest.assertEQ(str.lastIndexOf('ф', 2), 1);
    arktest.assertEQ(str.lastIndexOf('ф', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('ф', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('B'), 3);
    arktest.assertEQ(str.lastIndexOf('B', 3), 3);
    arktest.assertEQ(str.lastIndexOf('B', 4), 3);
    arktest.assertEQ(str.lastIndexOf('B', 2), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('ыB'), str.length - 'ыВы'.length);
    arktest.assertEQ(str.lastIndexOf('ыЯ'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('ыZ'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Bы'), str.length - 'Вы'.length);
    arktest.assertEQ(str.lastIndexOf('Bы', str.length - 'Вы'.length), str.length - 'Вы'.length);
    arktest.assertEQ(str.lastIndexOf('Бы'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Ab'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('ы'), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('ы', str.length), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('ы', str.length - 1), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('ы', 'АфыВы'.length - 1), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('ы', 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('ы', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('ы', 'Афы'.length), 'Аф'.length);
    arktest.assertEQ(str.lastIndexOf('ы', 'АфыВ'.length - 1), 'Аф'.length);

    str = 'AфыBн';
    arktest.assertEQ(str.lastIndexOf('AфыBнп'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('A'), 0);
    arktest.assertEQ(str.lastIndexOf('A', -1), 0);
    arktest.assertEQ(str.lastIndexOf('A', -str.length), 0);
    arktest.assertEQ(str.lastIndexOf('A', 1), 0);
    arktest.assertEQ(str.lastIndexOf('A', str.length), 0);
    arktest.assertEQ(str.lastIndexOf('A', str.length + 1), 0);
    arktest.assertEQ(str.lastIndexOf('AфыBн'), 0);
    arktest.assertEQ(str.lastIndexOf('AфыBм'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AЙыBн'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Aфы'), 0);
    arktest.assertEQ(str.lastIndexOf('Aфы', -1), 0);
    arktest.assertEQ(str.lastIndexOf('Aфы', 1), 0);
    arktest.assertEQ(str.lastIndexOf('Aфы', -str.length), 0);
    arktest.assertEQ(str.lastIndexOf('Aфы', str.length), 0);
    arktest.assertEQ(str.lastIndexOf('B'), str.length - 2);
    arktest.assertEQ(str.lastIndexOf('B', str.length - 'ыВн'.length), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('B', str.length - 'Вн'.length), str.length - 'Вн'.length);
    arktest.assertEQ(str.lastIndexOf('B', str.length - 'н'.length), str.length - 'Вн'.length);
    arktest.assertEQ(str.lastIndexOf('Bн'), str.length - 'Вн'.length);
    arktest.assertEQ(str.lastIndexOf('Bн', str.length - 'Вн'.length), str.length - 'Вн'.length);
    arktest.assertEQ(str.lastIndexOf('Вы'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('н'), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('н', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('н', -str.length), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('н', str.length - 'Вн'.length), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('н', str.length - 'н'.length), str.length - 'н'.length);
    arktest.assertEQ(str.lastIndexOf('н', str.length), str.length - 'н'.length);
    arktest.assertEQ(str.lastIndexOf('н', str.length + 1), str.length - 'н'.length);

    str = 'АфzWz';
    arktest.assertEQ(str.lastIndexOf('AфzWzп'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AbcdeG'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('А'), 0);
    arktest.assertEQ(str.lastIndexOf('А', -1), 0);
    arktest.assertEQ(str.lastIndexOf('А', -str.length), 0);
    arktest.assertEQ(str.lastIndexOf('А', 1), 0);
    arktest.assertEQ(str.lastIndexOf('А', str.length), 0);
    arktest.assertEQ(str.lastIndexOf('А', str.length + 1), 0);
    arktest.assertEQ(str.lastIndexOf('АфzWz'), 0);
    arktest.assertEQ(str.lastIndexOf('АфzWy'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('z'), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('z', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('z', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('z', 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('z', -str.length), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('z', str.length - 'zWz'.length), 'Афz'.length - 1);
    arktest.assertEQ(str.lastIndexOf('z', str.length - 'Wz'.length), 'Афz'.length - 1);
    arktest.assertEQ(str.lastIndexOf('z', str.length - 1), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('z', str.length), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('z', str.length + 1), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('AЙzWb'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('АфzW'), 0);
    arktest.assertEQ(str.lastIndexOf('АфzW', 0), 0);
    arktest.assertEQ(str.lastIndexOf('АфzW', -1), 0);
    arktest.assertEQ(str.lastIndexOf('АфzW', 1), 0);
    arktest.assertEQ(str.lastIndexOf('АфzW', str.length), 0);
    arktest.assertEQ(str.lastIndexOf('АфzW', str.length - 1), 0);
    arktest.assertEQ(str.lastIndexOf('АфzW', str.length + 1), 0);
    arktest.assertEQ(str.lastIndexOf('фzW'), 1);
    arktest.assertEQ(str.lastIndexOf('фzW', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('фzW', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('фzW', 1), 1);
    arktest.assertEQ(str.lastIndexOf('фzW', str.length - 'zWz'.length), 1);
    arktest.assertEQ(str.lastIndexOf('фzW', str.length - 'фzWz'.length), 1);
    arktest.assertEQ(str.lastIndexOf('фzW', str.length - 'Wz'.length), 1);
    arktest.assertEQ(str.lastIndexOf('ф'), 1);
    arktest.assertEQ(str.lastIndexOf('ф', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('ф', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('W'), str.length - 'Wz'.length);
    arktest.assertEQ(str.lastIndexOf('W', str.length - 'zWz'.length), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('W', str.length - 'Wz'.length), str.length - 'Wz'.length);
    arktest.assertEQ(str.lastIndexOf('W', str.length - 1), str.length - 'Wz'.length);
    arktest.assertEQ(str.lastIndexOf('Wz'), str.length - 'Wz'.length);
    arktest.assertEQ(str.lastIndexOf('Wz', str.length - 'Wz'.length), str.length - 'Wz'.length);
    arktest.assertEQ(str.lastIndexOf('Yz'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Wx'), CharNotFound);

    str = 'АфzWx';
    arktest.assertEQ(str.lastIndexOf('AфzWxп'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('zWy'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('А'), 0);
    arktest.assertEQ(str.lastIndexOf('А', -1), 0);
    arktest.assertEQ(str.lastIndexOf('А', -str.length), 0);
    arktest.assertEQ(str.lastIndexOf('А', 1), 0);
    arktest.assertEQ(str.lastIndexOf('А', str.length), 0);
    arktest.assertEQ(str.lastIndexOf('А', str.length - 1), 0);
    arktest.assertEQ(str.lastIndexOf('А', str.length + 1), 0);
    arktest.assertEQ(str.lastIndexOf('АфzWx'), 0);
    arktest.assertEQ(str.lastIndexOf('Афz'), 0);
    arktest.assertEQ(str.lastIndexOf('Афz', 0), 0);
    arktest.assertEQ(str.lastIndexOf('Афz', -1), 0);
    arktest.assertEQ(str.lastIndexOf('Афz', 1), 0);
    arktest.assertEQ(str.lastIndexOf('Афz', -str.length), 0);
    arktest.assertEQ(str.lastIndexOf('Афz', str.length), 0);
    arktest.assertEQ(str.lastIndexOf('Афz', str.length - 1), 0);
    arktest.assertEQ(str.lastIndexOf('Афz', str.length + 1), 0);
    arktest.assertEQ(str.lastIndexOf('АЙzWx'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('АфzWy'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('W'), str.length - 'Wx'.length);
    arktest.assertEQ(str.lastIndexOf('W', str.length - 'Wx'.length), str.length - 'Wx'.length);
    arktest.assertEQ(str.lastIndexOf('W', str.length - 1), str.length - 'Wx'.length);
    arktest.assertEQ(str.lastIndexOf('Wx'), str.length - 'Wx'.length);
    arktest.assertEQ(str.lastIndexOf('Wx', str.length - 'Wx'.length), str.length - 'Wx'.length);
    arktest.assertEQ(str.lastIndexOf('Wy'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('x'), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('x', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('x', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('x', str.length - 2), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('x', str.length - 1), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('x', str.length), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('x', str.length + 1), str.length - 1);
}

function testShortTreeString() {
    let str = ('super '.concat('--')).concat('alphA'.concat('!'));
    arktest.assertEQ(str.length, 14); // the minimum length of a tree string
    arktest.assertEQ(str.lastIndexOf('Aa'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('C'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Ц'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('A!'), str.length - 'A!'.length);
    arktest.assertEQ(str.lastIndexOf('A!', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('A!', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('A!', str.length - 'A!'.length - 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('A!', str.length - 'A!'.length), str.length - 'A!'.length);
    arktest.assertEQ(str.lastIndexOf('A!', str.length - 'A!'.length  + 1), str.length - 'A!'.length);
    arktest.assertEQ(str.lastIndexOf('A!', -str.length), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('A!', str.length), str.length - 'A!'.length);
    arktest.assertEQ(str.lastIndexOf('A!', str.length + 1), str.length - 'A!'.length);
    arktest.assertEQ(str.lastIndexOf('!'), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('!', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('!', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('!', str.length - 1), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('!', str.length - '!'.length - 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('!', str.length), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('A'), str.length - 'A!'.length);
    arktest.assertEQ(str.lastIndexOf('A', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('A', str.length - 'A!'.length - 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('A', str.length - 'A!'.length), str.length - 'A!'.length);
    arktest.assertEQ(str.lastIndexOf('A', str.length - 'A!'.length  + 1), str.length - 'A!'.length);
    arktest.assertEQ(str.lastIndexOf('A', -str.length), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('A', str.length), str.length - 'A!'.length);
    arktest.assertEQ(str.lastIndexOf('A', str.length + 1), str.length - 'A!'.length);
    arktest.assertEQ(str.lastIndexOf('-'), str.length - '-alphA!'.length);
    arktest.assertEQ(str.lastIndexOf('-', str.length - '-alphA!'.length), str.length - '-alphA!'.length);
    arktest.assertEQ(str.lastIndexOf('-', str.length - '--alphA!'.length), str.length - '--alphA!'.length);
    arktest.assertEQ(str.lastIndexOf('-', str.length - '--alphA!'.length - 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('s'), 0);
    arktest.assertEQ(str.lastIndexOf('s', -1), 0);
    arktest.assertEQ(str.lastIndexOf('s', -str.length), 0);
    arktest.assertEQ(str.lastIndexOf('s', 1), 0);
}

function testMiddleLinearString() {
    let str = 'ZyxWQqPpRrSsAaBb';
    arktest.assertEQ(str.lastIndexOf('Z'), 0);
    arktest.assertEQ(str.lastIndexOf('Z', -1), 0);
    arktest.assertEQ(str.lastIndexOf('Z', -str.length), 0);
    arktest.assertEQ(str.lastIndexOf('Z', 1), 0);
    arktest.assertEQ(str.lastIndexOf('Z', str.length), 0);
    arktest.assertEQ(str.lastIndexOf('Z', str.length + 1), 0);
    arktest.assertEQ(str.lastIndexOf('ZyxWQqPpRrSsAaBbc'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AaBb'), str.length - 'AaBb'.length);
    arktest.assertEQ(str.lastIndexOf('AaBb', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AaBb', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('ABBb'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AaBx'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('aBb'), str.length - 'aBb'.length);
    arktest.assertEQ(str.lastIndexOf('aBb', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('aBb', 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('aBb', str.length), str.length - 'aBb'.length);
    arktest.assertEQ(str.lastIndexOf('aBb', str.length - 'aBb'.length), str.length - 'aBb'.length);
    arktest.assertEQ(str.lastIndexOf('aBb', str.length - 'aBb'.length), str.length - 'aBb'.length);
    arktest.assertEQ(str.lastIndexOf('aBb', str.length - 'aBb'.length + 1), str.length - 'aBb'.length);
    arktest.assertEQ(str.lastIndexOf('aBb', str.length - 'aBb'.length - 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('aBb', str.length - 1), str.length - 'aBb'.length);
    arktest.assertEQ(str.lastIndexOf('aBb', str.length), str.length - 'aBb'.length);
    arktest.assertEQ(str.lastIndexOf('aXb'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Bx'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('!'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('b'), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('b', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('b', str.length - 2), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('b', str.length - 1), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('b', str.length), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('b', str.length + 1), str.length - 1);

    str = 'OpqrstuvwxzAaBab';
    arktest.assertEQ(str.lastIndexOf('O'), 0);
    arktest.assertEQ(str.lastIndexOf('O', -1), 0);
    arktest.assertEQ(str.lastIndexOf('O', -str.length), 0);
    arktest.assertEQ(str.lastIndexOf('O', 1), 0);
    arktest.assertEQ(str.lastIndexOf('O', str.length), 0);
    arktest.assertEQ(str.lastIndexOf('O', str.length + 1), 0);
    arktest.assertEQ(str.lastIndexOf('OpqrstuvwxzAaBabk'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AaBab'), str.length - 'AaBab'.length);
    arktest.assertEQ(str.lastIndexOf('AaBab', str.length - 'AaBab'.length), str.length - 'AaBab'.length);
    arktest.assertEQ(str.lastIndexOf('AaBab', str.length - 'AaBab'.length + 1), str.length - 'AaBab'.length);
    arktest.assertEQ(str.lastIndexOf('AaBab', str.length - 'AaBab'.length - 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AaBab', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AaBab', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('ABBab'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AaBaz'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('aBab'), str.length - 'aBab'.length);
    arktest.assertEQ(str.lastIndexOf('aBab', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('aBab', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('aBab', 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('aBab', str.length - 'aBab'.length), str.length - 'aBab'.length);
    arktest.assertEQ(str.lastIndexOf('aBab', str.length - 'AaBab'.length), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('aBab', str.length - 'Bab'.length), str.length - 'aBab'.length);
    arktest.assertEQ(str.lastIndexOf('aBab', str.length - 1), str.length - 'aBab'.length);
    arktest.assertEQ(str.lastIndexOf('aBab', str.length), str.length - 'aBab'.length);
    arktest.assertEQ(str.lastIndexOf('aBab', str.length + 1), str.length - 'aBab'.length);
    arktest.assertEQ(str.lastIndexOf('aX'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('!'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('aXab'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('a'), str.length - 'ab'.length);
    arktest.assertEQ(str.lastIndexOf('a', str.length - 'aBab'.length), str.length - 'aBab'.length);
    arktest.assertEQ(str.lastIndexOf('a', str.length - 'aBab'.length + 1), str.length - 'aBab'.length);
    arktest.assertEQ(str.lastIndexOf('a', str.length - 'aBab'.length - 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('a', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('a', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('a', 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('a', str.length - 1), str.length - 'ab'.length);
    arktest.assertEQ(str.lastIndexOf('a', str.length - 'ab'.length), str.length - 'ab'.length);
    arktest.assertEQ(str.lastIndexOf('a', str.length), str.length - 'ab'.length);
    arktest.assertEQ(str.lastIndexOf('a', str.length + 1), str.length - 'ab'.length);
    arktest.assertEQ(str.lastIndexOf('b'), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('b', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('b', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('b', str.length - 1), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('b', str.length), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('b', str.length + 1), str.length - 1);

    str = 'штзрЬоЯЯЙЫхAфыВы';
    arktest.assertEQ(str.lastIndexOf('ш'), 0);
    arktest.assertEQ(str.lastIndexOf('ш', -1), 0);
    arktest.assertEQ(str.lastIndexOf('ш', 0), 0);
    arktest.assertEQ(str.lastIndexOf('ш', -str.length), 0);
    arktest.assertEQ(str.lastIndexOf('ш', 1), 0);
    arktest.assertEQ(str.lastIndexOf('ш', str.length), 0);
    arktest.assertEQ(str.lastIndexOf('ш', str.length + 1), 0);
    arktest.assertEQ(str.lastIndexOf('штзрЬоЯЯЙЫхAфыВыщ'), -1);
    arktest.assertEQ(str.lastIndexOf('AфыВы'), str.length - 'АфыВы'.length);
    arktest.assertEQ(str.lastIndexOf('AЙbВ'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AЙыВц'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Вц'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Ц'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AфыВ'), str.length - 'АфыВы'.length);
    arktest.assertEQ(str.lastIndexOf('AфыВ', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AфыВ', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AфыВ', str.length - 'АфыВы'.length), str.length - 'АфыВы'.length);
    arktest.assertEQ(str.lastIndexOf('AфыВ', str.length - 'АфыВы'.length - 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AфыВ', str.length - 'АфыВы'.length + 1), str.length - 'АфыВы'.length);
    arktest.assertEQ(str.lastIndexOf('фыВ'), str.length - 'фыВы'.length);
    arktest.assertEQ(str.lastIndexOf('фыВ', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('фыВ', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('фыВ', 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('фыВ', str.length - 'фыВы'.length), str.length - 'фыВы'.length);
    arktest.assertEQ(str.lastIndexOf('фыВ', str.length - 'фыВы'.length - 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('фыВ', str.length - 'фыВы'.length + 1), str.length - 'фыВы'.length);
    arktest.assertEQ(str.lastIndexOf('фыВ', str.length - 1), str.length - 'фыВы'.length);
    arktest.assertEQ(str.lastIndexOf('фыВ', str.length), str.length - 'фыВы'.length);
    arktest.assertEQ(str.lastIndexOf('фыВ', str.length + 1), str.length - 'фыВы'.length);
    arktest.assertEQ(str.lastIndexOf('ф'), str.length - 'фыВы'.length);
    arktest.assertEQ(str.lastIndexOf('В'), str.length - 'Вы'.length);
    arktest.assertEQ(str.lastIndexOf('В', str.length - 'Вы'.length), str.length - 'Вы'.length);
    arktest.assertEQ(str.lastIndexOf('В', str.length - 'Вы'.length - 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('В', str.length - 'Вы'.length + 1), str.length - 'Вы'.length);
    arktest.assertEQ(str.lastIndexOf('Вы'), str.length - 'Вы'.length);
    arktest.assertEQ(str.lastIndexOf('Вы', str.length - 'Вы'.length), str.length - 'Вы'.length);
    arktest.assertEQ(str.lastIndexOf('Вы', str.length - 'Вы'.length - 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Вы', str.length - 'Вы'.length + 1), str.length - 'Вы'.length);
    arktest.assertEQ(str.lastIndexOf('Бы'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Ab'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('ы'), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('ы', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('ы', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('ы', str.length - 'фыВы'.length), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('ы', str.length - 'ыВы'.length), str.length - 'ыВы'.length);
    arktest.assertEQ(str.lastIndexOf('ы', str.length - 'Вы'.length), str.length - 'ыВы'.length);
    arktest.assertEQ(str.lastIndexOf('ы', str.length - 'ы'.length), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('ы', str.length), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('ы', str.length + 1), str.length - 1);

    str = 'штзрЬоЯЯЙЫхAфzWz';
    arktest.assertEQ(str.lastIndexOf('ш'), 0);
    arktest.assertEQ(str.lastIndexOf('ш', -1), 0);
    arktest.assertEQ(str.lastIndexOf('ш', 0), 0);
    arktest.assertEQ(str.lastIndexOf('ш', -str.length), 0);
    arktest.assertEQ(str.lastIndexOf('ш', 1), 0);
    arktest.assertEQ(str.lastIndexOf('ш', str.length), 0);
    arktest.assertEQ(str.lastIndexOf('ш', str.length + 1), 0);
    arktest.assertEQ(str.lastIndexOf('ш', str.length - 1), 0);
    arktest.assertEQ(str.lastIndexOf('штзрЬоЯЯЙЫхAфzWzщ'), -1);
    arktest.assertEQ(str.lastIndexOf('zWz'), str.length - 'zWz'.length);
    arktest.assertEQ(str.lastIndexOf('zWz', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('zWz', str.length), str.length - 'zWz'.length);
    arktest.assertEQ(str.lastIndexOf('zWz', str.length + 1), str.length - 'zWz'.length);
    arktest.assertEQ(str.lastIndexOf('zWz', str.length - 1), str.length - 'zWz'.length);
    arktest.assertEQ(str.lastIndexOf('zWz', str.length - 'zWz'.length), str.length - 'zWz'.length);
    arktest.assertEQ(str.lastIndexOf('zWz', str.length - 'фzWz'.length), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('zWz', str.length - 'Wz'.length), str.length - 'zWz'.length);
    arktest.assertEQ(str.lastIndexOf('zWx'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AфzWz'), str.length - 'АфzWz'.length);
    arktest.assertEQ(str.lastIndexOf('AЙbW'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AЙфzWц'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('хAфzWz'), str.length - 'хАфzWz'.length);
    arktest.assertEQ(str.lastIndexOf('хAфxWz'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('хAфzWx'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Wq'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Q'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AфzW'), str.length - 'АфzWz'.length);
    arktest.assertEQ(str.lastIndexOf('AфzW', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AфzW', str.length - 'АфzWz'.length), str.length - 'АфzWz'.length);
    arktest.assertEQ(str.lastIndexOf('AфzW', str.length - 'АфzWz'.length + 1), str.length - 'АфzWz'.length);
    arktest.assertEQ(str.lastIndexOf('AфzW', str.length - 'АфzWz'.length - 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('фzW'), str.length - 'фzWz'.length);
    arktest.assertEQ(str.lastIndexOf('фzW', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('фzW', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('фzW', 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('фzW', str.length - 'фzWz'.length), str.length - 'фzWz'.length);
    arktest.assertEQ(str.lastIndexOf('фzW', str.length - 'фzWz'.length - 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('фzW', str.length - 'фzWz'.length + 1), str.length - 'фzWz'.length);
    arktest.assertEQ(str.lastIndexOf('фzW', str.length - 1), str.length - 'фzWz'.length);
    arktest.assertEQ(str.lastIndexOf('фzW', str.length), str.length - 'фzWz'.length);
    arktest.assertEQ(str.lastIndexOf('ф'), str.length - 'фzWz'.length);
    arktest.assertEQ(str.lastIndexOf('W'), str.length - 'Wz'.length);
    arktest.assertEQ(str.lastIndexOf('W', str.length - 'Wz'.length), str.length - 'Wz'.length);
    arktest.assertEQ(str.lastIndexOf('W', str.length - 'Wz'.length - 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('W', str.length - 'Wz'.length + 1), str.length - 'Wz'.length);
    arktest.assertEQ(str.lastIndexOf('Wz'), str.length - 'Wz'.length);
    arktest.assertEQ(str.lastIndexOf('Vz'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Ab'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('z'), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('z', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('z', str.length - 'фzWz'.length), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('z', str.length - 'zWz'.length), str.length - 'zWz'.length);
    arktest.assertEQ(str.lastIndexOf('z', str.length - 'Wz'.length), str.length - 'zWz'.length);
    arktest.assertEQ(str.lastIndexOf('z', str.length - 1), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('z', str.length), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('z', str.length + 1), str.length - 1);

    str = 'ЗрвяСуНопрЩAфzWl';
    arktest.assertEQ(str.lastIndexOf('З'), 0);
    arktest.assertEQ(str.lastIndexOf('З', -1), 0);
    arktest.assertEQ(str.lastIndexOf('З', 0), 0);
    arktest.assertEQ(str.lastIndexOf('З', -str.length), 0);
    arktest.assertEQ(str.lastIndexOf('З', 1), 0);
    arktest.assertEQ(str.lastIndexOf('З', str.length), 0);
    arktest.assertEQ(str.lastIndexOf('З', str.length + 1), 0);
    arktest.assertEQ(str.lastIndexOf('З', str.length - 1), 0);
    arktest.assertEQ(str.lastIndexOf('ЗрвяСуНопрЩAфzWlч'), -1);
    arktest.assertEQ(str.lastIndexOf('AфzWl'), str.length - 'АфzWl'.length);
    arktest.assertEQ(str.lastIndexOf('AЙzWm'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AфzWm'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('AzWl'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('zWn'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('zWl'), str.length - 'zWl'.length);
    arktest.assertEQ(str.lastIndexOf('zWl', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('zWl', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('zWl', str.length - 'zWl'.length), str.length - 'zWl'.length);
    arktest.assertEQ(str.lastIndexOf('zWl', str.length - 'zWl'.length - 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('zWl', str.length - 'zWl'.length + 1), str.length - 'zWl'.length);
    arktest.assertEQ(str.lastIndexOf('W'), str.length - 'Wl'.length);
    arktest.assertEQ(str.lastIndexOf('W', str.length - 'Wl'.length), str.length - 'Wl'.length);
    arktest.assertEQ(str.lastIndexOf('W', str.length - 'Wl'.length + 1), str.length - 'Wl'.length);
    arktest.assertEQ(str.lastIndexOf('W', str.length - 'Wl'.length - 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Wl'), str.length - 'Wl'.length);
    arktest.assertEQ(str.lastIndexOf('Wl', str.length - 'Wl'.length), str.length - 'Wl'.length);
    arktest.assertEQ(str.lastIndexOf('Wl', str.length - 'Wl'.length + 1), str.length - 'Wl'.length);
    arktest.assertEQ(str.lastIndexOf('Wl', str.length - 'Wl'.length - 1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('Wm'), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('l'), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('l', -1), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('l', 0), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('l', str.length - 'Wl'.length), CharNotFound);
    arktest.assertEQ(str.lastIndexOf('l', str.length - 1), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('l', str.length), str.length - 1);
    arktest.assertEQ(str.lastIndexOf('l', str.length + 1), str.length - 1);
}

function testEscapedLinearString() {
    arktest.assertEQ(EscChars.lastIndexOf('\r\t\v\t', 1), CharNotFound);
    arktest.assertEQ(EscChars.lastIndexOf('\r\t\v', 1), CharNotFound);
    arktest.assertEQ(EscChars.lastIndexOf('\r\t\v', 2), 2);
    arktest.assertEQ(EscChars.lastIndexOf('\r\t\v', 3), 2);
    arktest.assertEQ(EscChars.lastIndexOf(EscChars), 0);
    arktest.assertEQ(EscChars.lastIndexOf(EscChars, -1), 0);
}

function testEscapedLinearSubstring() {
    testAllSubstrings(EscChars);
}

function testEscapedTreeSubstring() {
    let mixedTreeChars = EscChars.concat(EngChars);
    testAllSubstrings(mixedTreeChars);
}

function testEscapedSlicedSubstring() {
    let escSlicedChars = EscChars.substring(1, 20);
    testAllSubstrings(escSlicedChars);
}

function testNonCompressedLinearString() {
    arktest.assertEQ(RusChars.lastIndexOf('Ѫ'), CharNotFound);
    arktest.assertEQ(RusChars.lastIndexOf(RusChars), 0);
    arktest.assertEQ(RusChars.lastIndexOf(RusChars, RusChars.length), 0);
    arktest.assertEQ(RusChars.lastIndexOf(RusChars, 0), 0);
    arktest.assertEQ(RusChars.lastIndexOf(RusChars, -1), 0);
    arktest.assertEQ(RusChars.lastIndexOf(RusChars, RusChars.length - 1), 0);

    let RusCharsTooLongKey = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя_';
    arktest.assertEQ(RusChars.lastIndexOf(RusCharsTooLongKey), CharNotFound);

    let rusChars_1 = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let rusChars_2 = 'ОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя';
    arktest.assertEQ(RusChars.lastIndexOf(rusChars_1), 0);
    arktest.assertEQ(RusChars.lastIndexOf(rusChars_1, 0), 0);
    arktest.assertEQ(RusChars.lastIndexOf(rusChars_2), rusChars_1.length);
    arktest.assertEQ(RusChars.lastIndexOf(rusChars_2, rusChars_1.length), rusChars_1.length);
    arktest.assertEQ(RusChars.lastIndexOf(rusChars_2, rusChars_1.length + 1), rusChars_1.length);
    arktest.assertEQ(RusChars.lastIndexOf(rusChars_2, rusChars_1.length - 1), CharNotFound);

    let rusCharsShort = 'БбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    arktest.assertEQ(RusChars.lastIndexOf(rusCharsShort), 2);
    arktest.assertEQ(RusChars.lastIndexOf(rusCharsShort, 2), 2);
    arktest.assertEQ(RusChars.lastIndexOf(rusCharsShort, 3), 2);
    arktest.assertEQ(RusChars.lastIndexOf(rusCharsShort, 1), CharNotFound);
    arktest.assertEQ(RusChars.lastIndexOf(rusCharsShort, 0), CharNotFound);
    arktest.assertEQ(RusChars.lastIndexOf(rusCharsShort, -1), CharNotFound);
}

function testNonCompressedTreeString() {
    let rusChars_1 = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let rusChars_2 = 'ОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя';
    let rusTreeChars = rusChars_1.concat(rusChars_2);
    arktest.assertTrue(rusTreeChars === RusChars);
    arktest.assertTrue(RusChars === rusTreeChars);
    arktest.assertEQ(rusTreeChars.lastIndexOf(rusChars_1), 0);
    arktest.assertEQ(rusTreeChars.lastIndexOf(rusChars_2), rusChars_1.length);
    arktest.assertEQ(rusTreeChars.lastIndexOf(rusChars_2), rusChars_1.length);
    arktest.assertEQ(rusTreeChars.lastIndexOf(rusChars_2, rusChars_1.length), rusChars_1.length);
    arktest.assertEQ(rusTreeChars.lastIndexOf(rusChars_2, rusChars_1.length + 1), rusChars_1.length);
    arktest.assertEQ(rusTreeChars.lastIndexOf(rusChars_2, rusChars_1.length - 1), CharNotFound);
    arktest.assertEQ(rusTreeChars.lastIndexOf(rusChars_1 + '!'), CharNotFound);
    arktest.assertEQ(rusTreeChars.lastIndexOf(rusChars_1.concat('!')), CharNotFound);
    arktest.assertEQ(rusTreeChars.lastIndexOf(rusChars_2 + '!'), CharNotFound);
    arktest.assertEQ(rusTreeChars.lastIndexOf(rusChars_2.concat('!')), CharNotFound);
    arktest.assertEQ(RusChars.lastIndexOf(rusTreeChars), 0);
    arktest.assertEQ(RusChars.lastIndexOf(rusTreeChars, -1), 0);
    arktest.assertEQ(RusChars.lastIndexOf(rusTreeChars, rusTreeChars.length), 0);
    arktest.assertEQ(RusChars.lastIndexOf(rusTreeChars, rusTreeChars.length + 1), 0);

    let rusTreeChars2 = rusTreeChars.concat('!');
    arktest.assertEQ(rusTreeChars2.lastIndexOf(rusChars_1), 0);
    arktest.assertEQ(rusTreeChars2.lastIndexOf(rusChars_2), rusChars_1.length);
    arktest.assertEQ(rusTreeChars2.lastIndexOf(rusChars_2, rusChars_1.length), rusChars_1.length);
    arktest.assertEQ(rusTreeChars2.lastIndexOf(rusChars_2, rusChars_1.length + 1), rusChars_1.length);
    arktest.assertEQ(rusTreeChars2.lastIndexOf(rusChars_2, rusChars_1.length - 1), CharNotFound);
    arktest.assertEQ(rusTreeChars2.lastIndexOf(rusChars_2 + '!', rusChars_1.length + 1), rusChars_1.length);
    arktest.assertEQ(rusTreeChars2.lastIndexOf(rusChars_2.concat('!'), rusChars_1.length + 1), rusChars_1.length);
    arktest.assertEQ(rusTreeChars2.lastIndexOf(rusTreeChars2.concat('R')), CharNotFound);
    arktest.assertEQ(rusTreeChars2.lastIndexOf('R'.concat(rusTreeChars2)), CharNotFound);
}

function testNonCompressedSlicedString() {
    let rusChars_1 = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let rusChars_2 = 'ОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя';
    let rusTreeChars = rusChars_1.concat(rusChars_2);
    let rusSlicedChars = rusTreeChars.substring(3, 48);
    arktest.assertEQ(RusChars.lastIndexOf(rusSlicedChars), 3);
    arktest.assertEQ(rusSlicedChars.lastIndexOf(rusSlicedChars), 0);
    arktest.assertEQ(rusSlicedChars.lastIndexOf(rusSlicedChars, rusSlicedChars.length), 0);
    arktest.assertEQ(rusSlicedChars.lastIndexOf(rusSlicedChars, rusSlicedChars.length + 1), 0);
    arktest.assertEQ(rusSlicedChars.lastIndexOf(rusSlicedChars, rusSlicedChars.length - 1), 0);
    arktest.assertEQ(rusSlicedChars.lastIndexOf(rusSlicedChars, 0), 0);
    arktest.assertEQ(rusSlicedChars.lastIndexOf(rusSlicedChars, -1), 0);

    let rusCharsKey_1 = 'бВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    arktest.assertEQ(rusSlicedChars.lastIndexOf(rusCharsKey_1), 0);
    arktest.assertEQ(rusSlicedChars.lastIndexOf(rusCharsKey_1, rusCharsKey_1.length), 0);
    arktest.assertEQ(rusSlicedChars.lastIndexOf(rusCharsKey_1, rusCharsKey_1.length + 1), 0);
    arktest.assertEQ(rusSlicedChars.lastIndexOf(rusCharsKey_1, rusCharsKey_1.length - 1), 0);
    arktest.assertEQ(rusSlicedChars.lastIndexOf(rusCharsKey_1, 0), 0);
    arktest.assertEQ(rusSlicedChars.lastIndexOf(rusCharsKey_1, -1), 0);
    arktest.assertEQ(rusCharsKey_1, rusChars_1.substring(3, rusChars_1.length));
    arktest.assertEQ(rusSlicedChars.lastIndexOf(rusChars_1.substring(3, rusChars_1.length)), 0);

    let rusCharsKey_2 = 'ОоПпРрСсТтУуФфХхЦц';
    arktest.assertEQ(rusCharsKey_2, rusChars_2.substring(0, 18));
    arktest.assertEQ(rusSlicedChars.lastIndexOf(rusCharsKey_2), rusCharsKey_1.length);
    arktest.assertEQ(rusSlicedChars.lastIndexOf(rusCharsKey_2, rusCharsKey_1.length), rusCharsKey_1.length);
    arktest.assertEQ(rusSlicedChars.lastIndexOf(rusCharsKey_2, rusCharsKey_1.length + 1), rusCharsKey_1.length);
    arktest.assertEQ(rusSlicedChars.lastIndexOf(rusCharsKey_2, rusCharsKey_1.length - 1), CharNotFound);

    let rusTreeCharsTooLongKey = rusChars_1.concat(rusChars_2).concat('X');
    let rusSlicedCharsTooLongKey = rusTreeCharsTooLongKey.substring(3, 49);
    arktest.assertEQ(rusSlicedCharsTooLongKey.length, 46);
    arktest.assertEQ(rusSlicedChars.lastIndexOf(rusSlicedCharsTooLongKey), CharNotFound);
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
    arktest.assertEQ(EngChars.lastIndexOf(c'æ'),CharNotFound);
    arktest.assertEQ(EngChars.lastIndexOf('æ'), CharNotFound);
    arktest.assertEQ(EngChars.lastIndexOf(EngChars), 0);
    arktest.assertEQ(EngChars.lastIndexOf(EngChars, 0), 0);
    arktest.assertEQ(EngChars.lastIndexOf(EngChars, 1), 0);
    arktest.assertEQ(EngChars.lastIndexOf(EngChars, -1), 0);
    arktest.assertEQ(EngChars.lastIndexOf(EngChars, EngChars.length - 1), 0);

    let EngCharsShortKey = 'BbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz';
    arktest.assertEQ(EngChars.lastIndexOf(EngCharsShortKey), 2);
    arktest.assertEQ(EngChars.lastIndexOf(EngCharsShortKey, 2), 2);
    arktest.assertEQ(EngChars.lastIndexOf(EngCharsShortKey, 3), 2);
    arktest.assertEQ(EngChars.lastIndexOf(EngCharsShortKey, 1), CharNotFound);

    let EngCharsTooLongKey = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz_';
    arktest.assertEQ(EngChars.lastIndexOf(EngCharsTooLongKey), CharNotFound);
}

function testCompressedTreeString() {
    let engChars_1 = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNn';
    let engChars_2 = 'OoPpQqRrSsTtUuVvWwXxYyZz';
    let engTreeChars = engChars_1.concat(engChars_2);
    arktest.assertTrue(engTreeChars === EngChars);
    arktest.assertTrue(EngChars === engTreeChars);
    arktest.assertEQ(EngChars.lastIndexOf(engChars_1), 0);
    arktest.assertEQ(EngChars.lastIndexOf(engChars_2), engChars_1.length);
    arktest.assertEQ(EngChars.lastIndexOf(engChars_2, engChars_1.length), engChars_1.length);
    arktest.assertEQ(engTreeChars.lastIndexOf(engChars_1), 0);
    arktest.assertEQ(engTreeChars.lastIndexOf(engChars_2), engChars_1.length);
    arktest.assertEQ(engTreeChars.lastIndexOf(engChars_2, engChars_1.length), engChars_1.length);
    arktest.assertEQ(engTreeChars.lastIndexOf(engChars_2, engChars_1.length + 1), engChars_1.length);
    arktest.assertEQ(engTreeChars.lastIndexOf(engChars_2, engChars_1.length - 1), CharNotFound);
    arktest.assertEQ(engTreeChars.lastIndexOf(engChars_1 + '!'), CharNotFound);
    arktest.assertEQ(engTreeChars.lastIndexOf(engChars_1.concat('!')), CharNotFound);
    arktest.assertEQ(engTreeChars.lastIndexOf(engChars_2 + '!'), CharNotFound);
    arktest.assertEQ(engTreeChars.lastIndexOf(engChars_2.concat('!')), CharNotFound);
    arktest.assertEQ(engTreeChars.lastIndexOf(engTreeChars.concat('R')), CharNotFound);
    arktest.assertEQ(engTreeChars.lastIndexOf('R'.concat(engTreeChars)), CharNotFound);

    let engTreeChars2 = engTreeChars.concat('!');
    arktest.assertEQ(engTreeChars2.lastIndexOf(engChars_1), 0);
    arktest.assertEQ(engTreeChars2.lastIndexOf(engChars_2), engChars_1.length);
    arktest.assertEQ(engTreeChars2.lastIndexOf(engChars_2, engChars_1.length), engChars_1.length);
    arktest.assertEQ(engTreeChars2.lastIndexOf(engChars_2, engChars_1.length + 1), engChars_1.length);
    arktest.assertEQ(engTreeChars2.lastIndexOf(engChars_2, engChars_1.length - 1), CharNotFound);
    arktest.assertEQ(engTreeChars2.lastIndexOf(engChars_2 + '!', engChars_1.length + 1), engChars_1.length);
    arktest.assertEQ(engTreeChars2.lastIndexOf(engChars_2.concat('!'), engChars_1.length + 1), engChars_1.length);
    arktest.assertEQ(engTreeChars2.lastIndexOf(engTreeChars2.concat('R')), CharNotFound);
    arktest.assertEQ(engTreeChars2.lastIndexOf('R'.concat(engTreeChars2)), CharNotFound);
}

function testCompressedSlicedString() {
    let engChars_1 = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNn';
    let engChars_2 = 'OoPpQqRrSsTtUuVvWwXxYyZz';
    let engTreeChars = engChars_1.concat(engChars_2);
    let engSlicedChars = engTreeChars.substring(3, 43);
    arktest.assertEQ(EngChars.lastIndexOf(engSlicedChars), 3);
    arktest.assertEQ(engSlicedChars.lastIndexOf(engSlicedChars), 0);
    arktest.assertEQ(engSlicedChars.lastIndexOf(engSlicedChars, engSlicedChars.length), 0);
    arktest.assertEQ(engSlicedChars.lastIndexOf(engSlicedChars, engSlicedChars.length + 1), 0);
    arktest.assertEQ(engSlicedChars.lastIndexOf(engSlicedChars, 1), 0);
    arktest.assertEQ(engSlicedChars.lastIndexOf(engSlicedChars, 0), 0);
    arktest.assertEQ(engSlicedChars.lastIndexOf(engSlicedChars, -1), 0);

    let engCharsKey_1 = 'bCcDdEeFfGgHhIiJjKkLlMmNn';
    arktest.assertEQ(engSlicedChars.lastIndexOf(engCharsKey_1), 0);
    arktest.assertEQ(engSlicedChars.lastIndexOf(engCharsKey_1, engCharsKey_1.length), 0);
    arktest.assertEQ(engSlicedChars.lastIndexOf(engCharsKey_1, engCharsKey_1.length - 1), 0);
    arktest.assertEQ(engSlicedChars.lastIndexOf(engCharsKey_1, engCharsKey_1.length + 1), 0);
    arktest.assertEQ(engCharsKey_1, engChars_1.substring(3, engChars_1.length));
    arktest.assertEQ(engSlicedChars.lastIndexOf(engChars_1.substring(3, engChars_1.length)), 0);

    let engCharsKey_2 = 'OoPpQqRrSsTtUuV';
    arktest.assertEQ(engCharsKey_2, engChars_2.substring(0, 15));
    arktest.assertEQ(engSlicedChars.lastIndexOf(engCharsKey_2), engCharsKey_1.length);
    arktest.assertEQ(engSlicedChars.lastIndexOf(engCharsKey_2, engCharsKey_1.length), engCharsKey_1.length);
    arktest.assertEQ(engSlicedChars.lastIndexOf(engCharsKey_2, engCharsKey_1.length + 1), engCharsKey_1.length);
    arktest.assertEQ(engSlicedChars.lastIndexOf(engCharsKey_2, engCharsKey_1.length - 1), CharNotFound);

    let engTreeCharsTooLongKey = engChars_1.concat(engChars_2).concat('X');
    let engSlicedCharsTooLongKey = engTreeCharsTooLongKey.substring(3, 44);
    arktest.assertEQ(engSlicedCharsTooLongKey.length, 41);
    arktest.assertEQ(engSlicedChars.lastIndexOf(engSlicedCharsTooLongKey), CharNotFound);
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
    arktest.assertEQ(MixedChars.lastIndexOf(MixedChars), 0);
    arktest.assertEQ(MixedChars.lastIndexOf(MixedChars, MixedChars.length), 0);
    arktest.assertEQ(MixedChars.lastIndexOf(MixedChars, MixedChars.length + 1), 0);
    arktest.assertEQ(MixedChars.lastIndexOf(MixedChars, MixedChars.length + 1), 0);
    arktest.assertEQ(MixedChars.lastIndexOf(rusChars), 0);
    arktest.assertEQ(MixedChars.lastIndexOf(rusChars, 0), 0);
    arktest.assertEQ(MixedChars.lastIndexOf(rusChars, 1), 0);
    arktest.assertEQ(MixedChars.lastIndexOf(rusChars, -1), 0);
    arktest.assertEQ(MixedChars.lastIndexOf(engChars), rusChars.length);
    arktest.assertEQ(MixedChars.lastIndexOf(engChars, rusChars.length), rusChars.length);
    arktest.assertEQ(MixedChars.lastIndexOf(engChars, rusChars.length + 1), rusChars.length);
    arktest.assertEQ(MixedChars.lastIndexOf(engChars, rusChars.length - 1), CharNotFound);
    arktest.assertEQ(MixedChars.lastIndexOf(engChars, 1), CharNotFound);
    arktest.assertEQ(MixedChars.lastIndexOf(engChars, 0), CharNotFound);
    arktest.assertEQ(MixedChars.lastIndexOf(engChars, -1), CharNotFound);

    let rusCharsShortKey = 'БбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let mixedCharsShortKey = 'БбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнAaBbCcDdEeFfGgHhIiJjKkLl';
    arktest.assertEQ(MixedChars.lastIndexOf(rusCharsShortKey), 2);
    arktest.assertEQ(MixedChars.lastIndexOf(rusCharsShortKey, 2), 2);
    arktest.assertEQ(MixedChars.lastIndexOf(rusCharsShortKey, 1), CharNotFound);
    arktest.assertEQ(MixedChars.lastIndexOf(rusCharsShortKey, -1), CharNotFound);
    arktest.assertEQ(MixedChars.lastIndexOf(mixedCharsShortKey), 2);
    arktest.assertEQ(MixedChars.lastIndexOf(mixedCharsShortKey, 2), 2);
    arktest.assertEQ(MixedChars.lastIndexOf(mixedCharsShortKey, 1), CharNotFound);
    arktest.assertEQ(MixedChars.lastIndexOf(mixedCharsShortKey, -1), CharNotFound);

    let rusCharsLongKey = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн!';
    let mixedCharsLongKey = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнAaBbCcDdEeFfGgHhIiJjKkLl!';
    arktest.assertEQ(MixedChars.lastIndexOf(rusCharsLongKey), CharNotFound);
    arktest.assertEQ(MixedChars.lastIndexOf(mixedCharsLongKey), CharNotFound);
}

function testMixedCompressingTreeString() {
    let rusChars = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let engChars = 'AaBbCcDdEeFfGgHhIiJjKkLl';
    let mixedTreeChars = rusChars.concat(engChars);
    arktest.assertTrue(mixedTreeChars === MixedChars);
    arktest.assertTrue(MixedChars === mixedTreeChars);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(rusChars), 0);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(rusChars, 0), 0);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(rusChars, 1), 0);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(rusChars, -1), 0);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(rusChars, mixedTreeChars.length), 0);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(rusChars, rusChars.length), 0);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(rusChars, rusChars.length + 1), 0);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(rusChars, rusChars.length - 1), 0);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(engChars), rusChars.length);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(engChars, rusChars.length), rusChars.length);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(engChars, rusChars.length - 1), CharNotFound);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(engChars, -1), CharNotFound);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(engChars, mixedTreeChars.length), rusChars.length);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(engChars, mixedTreeChars.length - 1), rusChars.length);

    arktest.assertEQ(mixedTreeChars.lastIndexOf(rusChars, mixedTreeChars.length - 1), 0);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(engChars, rusChars.length - 1), CharNotFound);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(rusChars + '!'), CharNotFound);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(rusChars.concat('!')), CharNotFound);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(engChars + '!'), CharNotFound);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(engChars.concat('!')), CharNotFound);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(mixedTreeChars + '!'), CharNotFound);
    arktest.assertEQ(mixedTreeChars.lastIndexOf(mixedTreeChars.concat('!')), CharNotFound);

    let mixedTreeChars2 = mixedTreeChars.concat('!');
    arktest.assertEQ(mixedTreeChars2.lastIndexOf(rusChars), 0);
    arktest.assertEQ(mixedTreeChars2.lastIndexOf(rusChars, 0), 0);
    arktest.assertEQ(mixedTreeChars2.lastIndexOf(rusChars, 1), 0);
    arktest.assertEQ(mixedTreeChars2.lastIndexOf(rusChars, -1), 0);
    arktest.assertEQ(mixedTreeChars2.lastIndexOf(rusChars, mixedTreeChars2.length), 0);
    arktest.assertEQ(mixedTreeChars2.lastIndexOf(rusChars, rusChars.length), 0);
    arktest.assertEQ(mixedTreeChars2.lastIndexOf(rusChars, rusChars.length + 1), 0);
    arktest.assertEQ(mixedTreeChars2.lastIndexOf(rusChars, rusChars.length - 1), 0);
    arktest.assertEQ(mixedTreeChars2.lastIndexOf(rusChars + '!', rusChars.length + 1), CharNotFound);
    arktest.assertEQ(mixedTreeChars2.lastIndexOf(rusChars.concat('!'), rusChars.length + 1), CharNotFound);
    arktest.assertEQ(mixedTreeChars2.lastIndexOf(engChars), rusChars.length);
    arktest.assertEQ(mixedTreeChars2.lastIndexOf(engChars + '!'), rusChars.length);
    arktest.assertEQ(mixedTreeChars2.lastIndexOf(engChars.concat('!')), rusChars.length);
    arktest.assertEQ(mixedTreeChars2.lastIndexOf(engChars, rusChars.length), rusChars.length);
    arktest.assertEQ(mixedTreeChars2.lastIndexOf(engChars, rusChars.length - 1), CharNotFound);
    arktest.assertEQ(mixedTreeChars2.lastIndexOf(engChars, -1), CharNotFound);
    arktest.assertEQ(mixedTreeChars2.lastIndexOf(engChars, mixedTreeChars2.length), rusChars.length);
    arktest.assertEQ(mixedTreeChars2.lastIndexOf(engChars, mixedTreeChars.length - 1), rusChars.length);
}

function testMixedCompressingSlicedString() {
    let rusChars = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let engChars = 'OoPpQqRrSsTtUuVvWwXxYyZz';
    let mixedTreeChars = rusChars.concat(engChars);
    let mixedSlicedChars = mixedTreeChars.substring(3, rusChars.length + 15);
    arktest.assertEQ(mixedSlicedChars.length, rusChars.length + 15 - 3);
    let rusCharsKey = 'бВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    arktest.assertEQ(mixedSlicedChars.lastIndexOf(rusCharsKey), 0);
    arktest.assertEQ(mixedSlicedChars.lastIndexOf(rusCharsKey, 0), 0);
    arktest.assertEQ(mixedSlicedChars.lastIndexOf(rusCharsKey, 1), 0);
    arktest.assertEQ(mixedSlicedChars.lastIndexOf(rusCharsKey, -1), 0);
    arktest.assertEQ(mixedSlicedChars.lastIndexOf(rusCharsKey, rusCharsKey.length), 0);
    arktest.assertEQ(mixedSlicedChars.lastIndexOf(rusCharsKey, rusCharsKey.length - 1), 0);
    arktest.assertEQ(rusCharsKey, rusChars.substring(3, rusChars.length));
    arktest.assertEQ(mixedSlicedChars.lastIndexOf(rusChars.substring(3, rusChars.length)), 0);

    let engCharsKey = 'OoPpQqRrSsTtUuV';
    arktest.assertEQ(engCharsKey, engChars.substring(0, 15));
    arktest.assertEQ(mixedSlicedChars.lastIndexOf(engCharsKey), rusCharsKey.length);
    arktest.assertEQ(mixedSlicedChars.lastIndexOf(engCharsKey, rusCharsKey.length + 1), rusCharsKey.length);
    arktest.assertEQ(mixedSlicedChars.lastIndexOf(engCharsKey, rusCharsKey.length), rusCharsKey.length);
    arktest.assertEQ(mixedSlicedChars.lastIndexOf(engCharsKey, rusCharsKey.length - 1), CharNotFound);

    let mixedSlicedTooLongKey = (rusCharsKey.concat(engCharsKey).concat('!!!')).substring(1,
        rusCharsKey.length + engCharsKey.length + 2);
    arktest.assertLT(mixedSlicedChars.length, mixedSlicedTooLongKey.length);
    arktest.assertEQ(mixedSlicedChars.lastIndexOf(mixedSlicedTooLongKey), CharNotFound);
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
    let mixedTreeChars = Digits.concat(EngChars);
    testAllSubstrings(mixedTreeChars);
}

function testDigitsSlicedSubstring() {
    let digitsSlicedChars = Digits.substring(1, 8);
    arktest.assertEQ(digitsSlicedChars.length, 7);
    testAllSubstrings(digitsSlicedChars);
}

function testSymbolsLinearString() {
    arktest.assertEQ(Symbols.lastIndexOf(Symbols), 0);
    arktest.assertEQ(Symbols.lastIndexOf(Symbols, Symbols.length), 0);
    arktest.assertEQ(Symbols.lastIndexOf(Symbols, 0), 0);
    arktest.assertEQ(Symbols.lastIndexOf(Symbols, -1), 0);
    arktest.assertEQ(Symbols.lastIndexOf(Symbols, Symbols.length - 1), 0);
    arktest.assertEQ(Symbols.lastIndexOf(Symbols, Symbols.length + 1), 0);

    let halfOfSymbolsKey = '()_-+={}[]:;\"\'\\|/<>,.';
    arktest.assertEQ(Symbols.lastIndexOf(halfOfSymbolsKey), Symbols.length - halfOfSymbolsKey.length);
    arktest.assertEQ(Symbols.lastIndexOf(halfOfSymbolsKey, Symbols.length - halfOfSymbolsKey.length),
        Symbols.length - halfOfSymbolsKey.length);
    arktest.assertEQ(Symbols.lastIndexOf(halfOfSymbolsKey, Symbols.length - halfOfSymbolsKey.length - 1), CharNotFound);

    let symbolsCharsTooLongKey = '`~!@#$%^&*()_-+={}[]:;\"\'\\|/<>,.!';
    arktest.assertEQ(Symbols.lastIndexOf(symbolsCharsTooLongKey), CharNotFound);
}

function testSymbolsTreeString() {
    let symbols_1 = '`~!@#$%^&*()_-+={}[]';
    let symbols_2 = ':;\"\'\\|/<>,.';
    let symbolsTreeChars = symbols_1.concat(symbols_2);
    arktest.assertEQ(symbolsTreeChars.lastIndexOf(symbolsTreeChars), 0);
    arktest.assertEQ(symbolsTreeChars.lastIndexOf(symbolsTreeChars, 0), 0);
    arktest.assertEQ(symbolsTreeChars.lastIndexOf(symbolsTreeChars, 1), 0);
    arktest.assertEQ(symbolsTreeChars.lastIndexOf(symbolsTreeChars, -1), 0);
    arktest.assertEQ(symbolsTreeChars.lastIndexOf(symbolsTreeChars, symbolsTreeChars.length), 0);
    arktest.assertEQ(symbolsTreeChars.lastIndexOf(symbolsTreeChars, symbolsTreeChars.length - 1), 0);
    arktest.assertEQ(symbolsTreeChars.lastIndexOf(symbolsTreeChars, symbolsTreeChars.length + 1), 0);

    let halfOfSymbolsKey_1 = '()_-+={}[]:';
    let halfOfSymbolsKey_2 = ';\"\'\\|/<>,.';
    let halfOfSymbolsTreeKey = halfOfSymbolsKey_1.concat(halfOfSymbolsKey_2);
    arktest.assertEQ(Symbols.lastIndexOf(halfOfSymbolsTreeKey), Symbols.length - halfOfSymbolsTreeKey.length);
    arktest.assertEQ(Symbols.lastIndexOf(halfOfSymbolsTreeKey, Symbols.length - halfOfSymbolsTreeKey.length),
        Symbols.length - halfOfSymbolsTreeKey.length);
    arktest.assertEQ(Symbols.lastIndexOf(halfOfSymbolsTreeKey, Symbols.length - halfOfSymbolsTreeKey.length - 1),
        CharNotFound);

    let symbolsTreeCharsTooLongKey = symbolsTreeChars.concat('!');
    arktest.assertEQ(symbolsTreeChars.lastIndexOf(symbolsTreeCharsTooLongKey), CharNotFound);
}

function testSymbolsSlicedString() {
    let symbols_1 = '`~!@#$%^&*()_-+={}[]';
    let symbols_2 = ':;\"\'\\|/<>,.';
    let symbolsTreeChars = symbols_1.concat(symbols_2);
    let symbolsSlicedChars = symbolsTreeChars.substring(1, 14);
    arktest.assertEQ(symbolsSlicedChars.lastIndexOf(symbolsSlicedChars), 0);
    arktest.assertEQ(symbolsSlicedChars.lastIndexOf(symbolsSlicedChars, 0), 0);
    arktest.assertEQ(symbolsSlicedChars.lastIndexOf(symbolsSlicedChars, 1), 0);
    arktest.assertEQ(symbolsSlicedChars.lastIndexOf(symbolsSlicedChars, -1), 0);
    arktest.assertEQ(symbolsSlicedChars.lastIndexOf(symbolsSlicedChars, symbolsSlicedChars.length), 0);
    arktest.assertEQ(symbolsSlicedChars.lastIndexOf(symbolsSlicedChars, symbolsSlicedChars.length - 1), 0);
    arktest.assertEQ(symbolsSlicedChars.lastIndexOf(symbolsSlicedChars, symbolsSlicedChars.length + 1), 0);

    let halfOfSymbolsKey_1 = '()_-+={}[]:';
    let halfOfSymbolsKey_2 = ';\"\'\\|/<>,.!';
    let halfOfSymbolsTreeKey = halfOfSymbolsKey_1.concat(halfOfSymbolsKey_2);
    let halfOfSymbolsSlicedKey = halfOfSymbolsTreeKey.substring(1, halfOfSymbolsTreeKey.length - 1);
    arktest.assertEQ(Symbols.lastIndexOf(halfOfSymbolsSlicedKey), Symbols.length - halfOfSymbolsSlicedKey.length);
    arktest.assertEQ(Symbols.lastIndexOf(halfOfSymbolsSlicedKey, Symbols.length - halfOfSymbolsSlicedKey.length - 1),
        CharNotFound);

    let symbolsTreeCharsTooLongKey = symbolsTreeChars.concat('!!!');
    let symbolsSlicedCharsTooLongKey = symbolsTreeCharsTooLongKey.substring(1, 15);
    arktest.assertEQ(symbolsSlicedCharsTooLongKey.length, 14);
    arktest.assertEQ(symbolsSlicedChars.lastIndexOf(symbolsSlicedCharsTooLongKey), CharNotFound);
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

function testVeryLongMixedCompressingLinearString() {
    let longMixedChars: String = 'нфUghДвгатфLpfdjdOklbZodissborofjЗзИиЙйКкЛлМмНнweflkwlщцшуддцйзжцйэIiJjKkLlйцюцйффыогцуоцуюдйцжйжцхэpwpqwKJisisiYIUЮКОйцьйцйжцхэpwpqwKYIUЮКОйцЮКОйц|АаБбВвГгДдЕеЁёЖжAaBbCcDdEeFfGgHh';
    arktest.assertEQ(180, longMixedChars.length)
    let rusChars = 'АаБбВвГгДдЕеЁёЖж';
    let engChars = 'AaBbCcDdEeFfGgHh';
    let mixedPrefix = 'нфUgh';
    arktest.assertEQ(longMixedChars.lastIndexOf(longMixedChars), 0);
    arktest.assertEQ(longMixedChars.lastIndexOf(longMixedChars, 0), 0);
    arktest.assertEQ(longMixedChars.lastIndexOf(longMixedChars, longMixedChars.length), 0);
    arktest.assertEQ(longMixedChars.lastIndexOf(longMixedChars, longMixedChars.length + 1), 0);
    arktest.assertEQ(longMixedChars.lastIndexOf(longMixedChars, longMixedChars.length - 1), 0);
    arktest.assertEQ(longMixedChars.lastIndexOf(mixedPrefix, 0), 0);
    arktest.assertEQ(longMixedChars.lastIndexOf(mixedPrefix, 1), 0);
    arktest.assertEQ(longMixedChars.lastIndexOf(mixedPrefix, -1), 0);
    arktest.assertEQ(longMixedChars.lastIndexOf(rusChars),
        longMixedChars.length - rusChars.length - engChars.length);
    arktest.assertEQ(longMixedChars.lastIndexOf(rusChars, 0), CharNotFound);
    arktest.assertEQ(longMixedChars.lastIndexOf(rusChars, 1), CharNotFound);
    arktest.assertEQ(longMixedChars.lastIndexOf(rusChars, -1), CharNotFound);
    arktest.assertEQ(longMixedChars.lastIndexOf(engChars),
        longMixedChars.length - rusChars.length - engChars.length + rusChars.length);
    arktest.assertEQ(longMixedChars.lastIndexOf(engChars, longMixedChars.length - engChars.length),
        longMixedChars.length - rusChars.length - engChars.length + rusChars.length);
    arktest.assertEQ(longMixedChars.lastIndexOf(engChars, longMixedChars.length - engChars.length + 1),
        longMixedChars.length - rusChars.length - engChars.length + rusChars.length);
    arktest.assertEQ(longMixedChars.lastIndexOf(engChars, longMixedChars.length - engChars.length - 1), CharNotFound);
    arktest.assertEQ(longMixedChars.lastIndexOf(engChars, 1), CharNotFound);
    arktest.assertEQ(longMixedChars.lastIndexOf(engChars, 0), CharNotFound);
    arktest.assertEQ(longMixedChars.lastIndexOf(engChars, -1), CharNotFound);

    let longMixedCharsPlus: String = 'нфUghДвгатфLpfdjdOklbZodissborofjЗзИиЙйКкЛлМмНнweflkwlщцшуддцйзжцйэIiJjKkLlйцюцйффыогцуоцуюдйцжйжцхэpwpqwKJisisiYIUЮКОйцьйцйжцхэpwpqwKYIUЮКОйцЮКОйц|АаБбВвГгДдЕеЁёЖжAaBbCcDdEeFfGgHh!';
    arktest.assertLT(180, longMixedCharsPlus.length)
    arktest.assertEQ(longMixedCharsPlus.lastIndexOf(longMixedChars), 0);
    arktest.assertEQ(longMixedCharsPlus.lastIndexOf(longMixedChars, 0), 0);
    arktest.assertEQ(longMixedCharsPlus.lastIndexOf(longMixedChars, 1), 0);
    arktest.assertEQ(longMixedCharsPlus.lastIndexOf(longMixedChars, longMixedChars.length), 0);
    arktest.assertEQ(longMixedCharsPlus.lastIndexOf(longMixedChars, longMixedChars.length - 1), 0);
    arktest.assertEQ(longMixedCharsPlus.lastIndexOf(mixedPrefix, 0), 0);
    arktest.assertEQ(longMixedCharsPlus.lastIndexOf(mixedPrefix, 1), 0);
    arktest.assertEQ(longMixedCharsPlus.lastIndexOf(mixedPrefix, -1), 0);
    arktest.assertEQ(longMixedCharsPlus.lastIndexOf(rusChars),
        longMixedChars.length - rusChars.length - engChars.length);
    arktest.assertEQ(longMixedCharsPlus.lastIndexOf(engChars),
        longMixedChars.length - rusChars.length - engChars.length + rusChars.length);
}

function main(): int {
    /**
     * lastIndexOf(string): int => StdCoreStringlastIndexOfString
     */
    let suite = new arktest.ArkTestsuite('StringLastIndexOfString');
    suite.addTest('testEmptyString', testEmptyString);
    suite.addTest('testTreeOneCharString', testTreeOneCharString);
    suite.addTest('testEscapedLinearString', testEscapedLinearString);
    suite.addTest('testEscapedLinearSubstring', testEscapedLinearSubstring);
    suite.addTest('testEscapedTreeSubstring', testEscapedTreeSubstring);
    suite.addTest('testEscapedSlicedSubstring', testEscapedSlicedSubstring);
    suite.addTest('testShortLinearString', testShortLinearString);
    suite.addTest('testShortTreeString', testShortTreeString);
    suite.addTest('testMiddleLinearString', testMiddleLinearString);
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
    suite.addTest('testVeryLongMixedCompressingLinearString', testVeryLongMixedCompressingLinearString);
    return suite.run();
}
