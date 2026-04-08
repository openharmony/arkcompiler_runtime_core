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
    let str = 'A';
    arktest.assertEQ(str.indexOf('Aa'), CharNotFound);
    arktest.assertEQ(str.indexOf('C'), CharNotFound);
    arktest.assertEQ(str.indexOf('Ц'), CharNotFound);
    arktest.assertEQ(str.indexOf('A'), 0);
    arktest.assertEQ(str.indexOf('A', 0), 0);
    arktest.assertEQ(str.indexOf('A', -str.length), 0);
    arktest.assertEQ(str.indexOf('A', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('A', str.length + 1), CharNotFound);

    str = 'AaBb';
    arktest.assertEQ(str.indexOf('AaBbx'), CharNotFound);
    arktest.assertEQ(str.indexOf('A'), 0);
    arktest.assertEQ(str.indexOf('Ц'), CharNotFound);
    arktest.assertEQ(str.indexOf('A', 0), 0);
    arktest.assertEQ(str.indexOf('A', -1), 0);
    arktest.assertEQ(str.indexOf('A', -str.length), 0);
    arktest.assertEQ(str.indexOf('A', 1), CharNotFound);
    arktest.assertEQ(str.indexOf('A', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('A', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('AaBb'), 0);
    arktest.assertEQ(str.indexOf('AaBb', 0), 0);
    arktest.assertEQ(str.indexOf('AaBb', -1), 0);
    arktest.assertEQ(str.indexOf('AaBz'), CharNotFound);
    arktest.assertEQ(str.indexOf('ABBb'), CharNotFound);
    arktest.assertEQ(str.indexOf('aBb'), 1);
    arktest.assertEQ(str.indexOf('aBb', 0), 1);
    arktest.assertEQ(str.indexOf('aBb', -1), 1);
    arktest.assertEQ(str.indexOf('aBb', 1), 1);
    arktest.assertEQ(str.indexOf('aBb', 2), CharNotFound);
    arktest.assertEQ(str.indexOf('aXb'), CharNotFound);
    arktest.assertEQ(str.indexOf('b'), str.length - 1);
    arktest.assertEQ(str.indexOf('b', 0), str.length - 1);
    arktest.assertEQ(str.indexOf('b', -1), str.length - 1);
    arktest.assertEQ(str.indexOf('b', -str.length), str.length - 1);
    arktest.assertEQ(str.indexOf('b', str.length - 2), str.length - 1);
    arktest.assertEQ(str.indexOf('b', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf('b', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('b', str.length + 1), CharNotFound);

    str = 'AaBab';
    arktest.assertEQ(str.indexOf('AaBabx'), CharNotFound);
    arktest.assertEQ(str.indexOf('A'), 0);
    arktest.assertEQ(str.indexOf('A', 0), 0);
    arktest.assertEQ(str.indexOf('A', -1), 0);
    arktest.assertEQ(str.indexOf('A', -str.length), 0);
    arktest.assertEQ(str.indexOf('A', 1), CharNotFound);
    arktest.assertEQ(str.indexOf('A', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('A', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('AaBab'), 0);
    arktest.assertEQ(str.indexOf('AaBab', 0), 0);
    arktest.assertEQ(str.indexOf('AaBab', -1), 0);
    arktest.assertEQ(str.indexOf('AaBaz'), CharNotFound);
    arktest.assertEQ(str.indexOf('ABBab'), CharNotFound);
    arktest.assertEQ(str.indexOf('aBab'), 1);
    arktest.assertEQ(str.indexOf('aBab', -1), 1);
    arktest.assertEQ(str.indexOf('aBab', 1), 1);
    arktest.assertEQ(str.indexOf('aBab', 2), CharNotFound);
    arktest.assertEQ(str.indexOf('aXab'), CharNotFound);
    arktest.assertEQ(str.indexOf('Ba', 0), str.length - 'Bab'.length);
    arktest.assertEQ(str.indexOf('Ba'), str.length - 'Bab'.length);
    arktest.assertEQ(str.indexOf('Bb'), CharNotFound);
    arktest.assertEQ(str.indexOf('a'), 1);
    arktest.assertEQ(str.indexOf('a', 0), 1);
    arktest.assertEQ(str.indexOf('a', 1), 1);
    arktest.assertEQ(str.indexOf('a', -1), 1);
    arktest.assertEQ(str.indexOf('a', str.length - 1), CharNotFound);
    arktest.assertEQ(str.indexOf('a', str.length - 2), str.length - 2);
    arktest.assertEQ(str.indexOf('a', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('a', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('b'), str.length - 1);
    arktest.assertEQ(str.indexOf('b', 0), str.length - 1);
    arktest.assertEQ(str.indexOf('b', -1), str.length - 1);
    arktest.assertEQ(str.indexOf('b', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf('b', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('b', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('ц'), CharNotFound);

    str = 'A';
    arktest.assertEQ(str.indexOf('Aб'), CharNotFound);
    arktest.assertEQ(str.indexOf('б'), CharNotFound);
    arktest.assertEQ(str.indexOf('A'), 0);
    arktest.assertEQ(str.indexOf('A', 0), 0);
    arktest.assertEQ(str.indexOf('A', -str.length), 0);
    arktest.assertEQ(str.indexOf('A', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('A', str.length + 1), CharNotFound);

    str = 'AфыBы';
    arktest.assertEQ(str.indexOf('AфыBып'), CharNotFound);
    arktest.assertEQ(str.indexOf('A'), 0);
    arktest.assertEQ(str.indexOf('A', 0), 0);
    arktest.assertEQ(str.indexOf('A', -1), 0);
    arktest.assertEQ(str.indexOf('A', -str.length), 0);
    arktest.assertEQ(str.indexOf('A', 1), CharNotFound);
    arktest.assertEQ(str.indexOf('A', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('A', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('AфыBы'), 0);
    arktest.assertEQ(str.indexOf('AЙыBb'), CharNotFound);
    arktest.assertEQ(str.indexOf('AфыBя'), CharNotFound);
    arktest.assertEQ(str.indexOf('AфыB'), 0);
    arktest.assertEQ(str.indexOf('AфыB', 0), 0);
    arktest.assertEQ(str.indexOf('AфыB', -1), 0);
    arktest.assertEQ(str.indexOf('AфыB', 1), CharNotFound);
    arktest.assertEQ(str.indexOf('фыB'), 1);
    arktest.assertEQ(str.indexOf('фыB', -1), 1);
    arktest.assertEQ(str.indexOf('фыB', 1), 1);
    arktest.assertEQ(str.indexOf('фыB', 2), CharNotFound);
    arktest.assertEQ(str.indexOf('фыB', 3), CharNotFound);
    arktest.assertEQ(str.indexOf('ф', 0), 1);
    arktest.assertEQ(str.indexOf('ф'), 1);
    arktest.assertEQ(str.indexOf('B'), 3);
    arktest.assertEQ(str.indexOf('B', 0), 3);
    arktest.assertEQ(str.indexOf('B', 3), 3);
    arktest.assertEQ(str.indexOf('B', str.length - 1), CharNotFound);
    arktest.assertEQ(str.indexOf('ыB'), str.length - 'ыВы'.length);
    arktest.assertEQ(str.indexOf('ыЯ'), CharNotFound);
    arktest.assertEQ(str.indexOf('ыZ'), CharNotFound);
    arktest.assertEQ(str.indexOf('Bы'), 3);
    arktest.assertEQ(str.indexOf('Bы', 3), 3);
    arktest.assertEQ(str.indexOf('Бы'), CharNotFound);
    arktest.assertEQ(str.indexOf('Вц'), CharNotFound);
    arktest.assertEQ(str.indexOf('ы'), 2);
    arktest.assertEQ(str.indexOf('ы', 0), 2);
    arktest.assertEQ(str.indexOf('ы', -1), 2);
    arktest.assertEQ(str.indexOf('ы', str.length - 2), str.length - 1);
    arktest.assertEQ(str.indexOf('ы', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf('ы', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('ы', str.length + 1), CharNotFound);

    str = 'AфыBн';
    arktest.assertEQ(str.indexOf('AфыBнп'), CharNotFound);
    arktest.assertEQ(str.indexOf('A'), 0);
    arktest.assertEQ(str.indexOf('A', 0), 0);
    arktest.assertEQ(str.indexOf('A', -1), 0);
    arktest.assertEQ(str.indexOf('A', -str.length), 0);
    arktest.assertEQ(str.indexOf('A', 1), CharNotFound);
    arktest.assertEQ(str.indexOf('A', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('A', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('AфыBн'), 0);
    arktest.assertEQ(str.indexOf('AфыBн', 0), 0);
    arktest.assertEQ(str.indexOf('AфыBм'), CharNotFound);
    arktest.assertEQ(str.indexOf('AЙыBн'), CharNotFound);
    arktest.assertEQ(str.indexOf('B'), 3);
    arktest.assertEQ(str.indexOf('B', 0), 3);
    arktest.assertEQ(str.indexOf('B', str.length - 'ыВн'.length), str.length - 'Вн'.length);
    arktest.assertEQ(str.indexOf('B', str.length - 'Вн'.length), str.length - 'Вн'.length);
    arktest.assertEQ(str.indexOf('B', str.length - 'н'.length), CharNotFound);
    arktest.assertEQ(str.indexOf('Bн'), str.length - 'Вн'.length);
    arktest.assertEQ(str.indexOf('Bн', str.length - 'Вн'.length), str.length - 'Вн'.length);
    arktest.assertEQ(str.indexOf('Bн', str.length - 'ыВн'.length), str.length - 'Вн'.length);
    arktest.assertEQ(str.indexOf('Вы'), CharNotFound);
    arktest.assertEQ(str.indexOf('н'), str.length - 1);
    arktest.assertEQ(str.indexOf('н', 0), str.length - 1);
    arktest.assertEQ(str.indexOf('н', -1), str.length - 1);
    arktest.assertEQ(str.indexOf('н', str.length - 2), str.length - 1);
    arktest.assertEQ(str.indexOf('н', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf('н', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('н', str.length + 1), CharNotFound);

    str = 'АфzWz';
    arktest.assertEQ(str.indexOf('AфzWzп'), CharNotFound);
    arktest.assertEQ(str.indexOf('AbcdeG'), CharNotFound);
    arktest.assertEQ(str.indexOf('А'), 0);
    arktest.assertEQ(str.indexOf('А', 0), 0);
    arktest.assertEQ(str.indexOf('А', -1), 0);
    arktest.assertEQ(str.indexOf('А', -str.length), 0);
    arktest.assertEQ(str.indexOf('А', 1), CharNotFound);
    arktest.assertEQ(str.indexOf('А', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('А', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('АфzWz'), 0);
    arktest.assertEQ(str.indexOf('АфzWz', 0), 0);
    arktest.assertEQ(str.indexOf('АфzWy'), CharNotFound);
    arktest.assertEQ(str.indexOf('z'), 2);
    arktest.assertEQ(str.indexOf('z', 0), 2);
    arktest.assertEQ(str.indexOf('z', -1), 2);
    arktest.assertEQ(str.indexOf('z', 1), 2);
    arktest.assertEQ(str.indexOf('z', -str.length), 2);
    arktest.assertEQ(str.indexOf('z', str.length - 2), str.length - 1);
    arktest.assertEQ(str.indexOf('z', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf('z', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('z', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('AЙzWb'), CharNotFound);
    arktest.assertEQ(str.indexOf('АфzW'), 0);
    arktest.assertEQ(str.indexOf('АфzW', 0), 0);
    arktest.assertEQ(str.indexOf('АфzW', -1), 0);
    arktest.assertEQ(str.indexOf('АфzW', 1), CharNotFound);
    arktest.assertEQ(str.indexOf('фzW'), 1);
    arktest.assertEQ(str.indexOf('фzW', -1), 1);
    arktest.assertEQ(str.indexOf('фzW', 0), 1);
    arktest.assertEQ(str.indexOf('фzW', 1), 1);
    arktest.assertEQ(str.indexOf('фzW', 2), CharNotFound);
    arktest.assertEQ(str.indexOf('фzW', 3), CharNotFound);
    arktest.assertEQ(str.indexOf('ф'), 1);
    arktest.assertEQ(str.indexOf('ф', 0), 1);
    arktest.assertEQ(str.indexOf('W'), 3);
    arktest.assertEQ(str.indexOf('W', 0), 3);
    arktest.assertEQ(str.indexOf('W', 3), 3);
    arktest.assertEQ(str.indexOf('W', str.length - 1), CharNotFound);
    arktest.assertEQ(str.indexOf('Wz'), 3);
    arktest.assertEQ(str.indexOf('Wz', 0), 3);
    arktest.assertEQ(str.indexOf('Wz', 3), 3);
    arktest.assertEQ(str.indexOf('Yz'), CharNotFound);
    arktest.assertEQ(str.indexOf('Wx'), CharNotFound);

    str = 'АфzWx';
    arktest.assertEQ(str.indexOf('AфzWxп'), CharNotFound);
    arktest.assertEQ(str.indexOf('zWy'), CharNotFound);
    arktest.assertEQ(str.indexOf('А'), 0);
    arktest.assertEQ(str.indexOf('А', 0), 0);
    arktest.assertEQ(str.indexOf('А', -1), 0);
    arktest.assertEQ(str.indexOf('А', -str.length), 0);
    arktest.assertEQ(str.indexOf('А', 1), CharNotFound);
    arktest.assertEQ(str.indexOf('А', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('А', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('АфzWx'), 0);
    arktest.assertEQ(str.indexOf('АфzWx', 0), 0);
    arktest.assertEQ(str.indexOf('АЙzWx'), CharNotFound);
    arktest.assertEQ(str.indexOf('АфzWy'), CharNotFound);
    arktest.assertEQ(str.indexOf('W'), 3);
    arktest.assertEQ(str.indexOf('W', 0), 3);
    arktest.assertEQ(str.indexOf('W', 3), 3);
    arktest.assertEQ(str.indexOf('W', str.length - 1), CharNotFound);
    arktest.assertEQ(str.indexOf('Wx'), 3);
    arktest.assertEQ(str.indexOf('Wx', 0), 3);
    arktest.assertEQ(str.indexOf('Wx', 3), 3);
    arktest.assertEQ(str.indexOf('Wy'), CharNotFound);
    arktest.assertEQ(str.indexOf('x'), str.length - 1);
    arktest.assertEQ(str.indexOf('x', 0), str.length - 1);
    arktest.assertEQ(str.indexOf('x', -1), str.length - 1);
    arktest.assertEQ(str.indexOf('x', str.length - 2), str.length - 1);
    arktest.assertEQ(str.indexOf('x', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf('x', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('x', str.length + 1), CharNotFound);
}

function testShortTreeString() {
    let str = ('super '.concat('--')).concat('alphA'.concat('!'));
    arktest.assertEQ(str.length, 14); // the minimum length of a tree string
    arktest.assertEQ(str.indexOf('Aa'), CharNotFound);
    arktest.assertEQ(str.indexOf('C'), CharNotFound);
    arktest.assertEQ(str.indexOf('Ц'), CharNotFound);
    arktest.assertEQ(str.indexOf('A!'), str.length - 'A!'.length);
    arktest.assertEQ(str.indexOf('A!', 0), str.length - 'A!'.length);
    arktest.assertEQ(str.indexOf('A!', str.length - 'A!'.length - 1), str.length - 'A!'.length);
    arktest.assertEQ(str.indexOf('A!', str.length - 'A!'.length), str.length - 'A!'.length);
    arktest.assertEQ(str.indexOf('A!', str.length - 'A!'.length  + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('A!', -str.length), str.length - 'A!'.length);
    arktest.assertEQ(str.indexOf('A!', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('A!', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('!'), str.length - 1);
    arktest.assertEQ(str.indexOf('!', 0), str.length - 1);
    arktest.assertEQ(str.indexOf('!', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf('!', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('A'), str.length - 'A!'.length);
    arktest.assertEQ(str.indexOf('A', 0), str.length - 'A!'.length);
    arktest.assertEQ(str.indexOf('A', str.length - 'A!'.length - 1), str.length - 'A!'.length);
    arktest.assertEQ(str.indexOf('A', str.length - 'A!'.length), str.length - 'A!'.length);
    arktest.assertEQ(str.indexOf('A', str.length - 'A!'.length  + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('A', -str.length), str.length - 'A!'.length);
    arktest.assertEQ(str.indexOf('A', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('A', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('-'), str.length - '--alphA!'.length);
    arktest.assertEQ(str.indexOf('-', str.length - '--alphA!'.length), str.length - '--alphA!'.length);
    arktest.assertEQ(str.indexOf('-', str.length - '-alphA!'.length), str.length - '-alphA!'.length);
    arktest.assertEQ(str.indexOf('s'), 0);
    arktest.assertEQ(str.indexOf('s', -1), 0);
    arktest.assertEQ(str.indexOf('s', -str.length), 0);
    arktest.assertEQ(str.indexOf('s', 1), CharNotFound);
}

function testMiddleLinearString() {
    let str = 'ZyxWQqPpRrSsAaBb';
    arktest.assertEQ(str.indexOf('Z'), 0);
    arktest.assertEQ(str.indexOf('Z', -1), 0);
    arktest.assertEQ(str.indexOf('Z', -str.length), 0);
    arktest.assertEQ(str.indexOf('Z', 1), CharNotFound);
    arktest.assertEQ(str.indexOf('Z', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('Z', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('ZyxWQqPpRrSsAaBbc'), CharNotFound);
    arktest.assertEQ(str.indexOf('AaBb'), str.length - 'AaBb'.length);
    arktest.assertEQ(str.indexOf('AaBb', -1), str.length - 'AaBb'.length);
    arktest.assertEQ(str.indexOf('ABBb'), CharNotFound);
    arktest.assertEQ(str.indexOf('AaBx'), CharNotFound);
    arktest.assertEQ(str.indexOf('aBb'), str.length - 'aBb'.length);
    arktest.assertEQ(str.indexOf('aBb', -1), str.length - 'aBb'.length);
    arktest.assertEQ(str.indexOf('aBb', 1), str.length - 'aBb'.length);
    arktest.assertEQ(str.indexOf('aBb', str.length - 'aBb'.length), str.length - 'aBb'.length);
    arktest.assertEQ(str.indexOf('aBb', str.length - 'aBb'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('aBb', str.length - 1), CharNotFound);
    arktest.assertEQ(str.indexOf('aBb', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('aXb'), CharNotFound);
    arktest.assertEQ(str.indexOf('Bx'), CharNotFound);
    arktest.assertEQ(str.indexOf('!'), CharNotFound);
    arktest.assertEQ(str.indexOf('b'), str.length - 1);
    arktest.assertEQ(str.indexOf('b', -1), str.length - 1);
    arktest.assertEQ(str.indexOf('b', str.length - 2), str.length - 1);
    arktest.assertEQ(str.indexOf('b', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf('b', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('b', str.length + 1), CharNotFound);

    str = 'OpqrstuvwxzAaBab';
    arktest.assertEQ(str.indexOf('O'), 0);
    arktest.assertEQ(str.indexOf('O', -1), 0);
    arktest.assertEQ(str.indexOf('O', -str.length), 0);
    arktest.assertEQ(str.indexOf('O', 1), CharNotFound);
    arktest.assertEQ(str.indexOf('O', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('O', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('OpqrstuvwxzAaBabk'), CharNotFound);
    arktest.assertEQ(str.indexOf('AaBab'), str.length - 'AaBab'.length);
    arktest.assertEQ(str.indexOf('AaBab', str.length - 'AaBab'.length), str.length - 'AaBab'.length);
    arktest.assertEQ(str.indexOf('AaBab', str.length - 'AaBab'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('AaBab', -1), str.length - 'AaBab'.length);
    arktest.assertEQ(str.indexOf('ABBab'), CharNotFound);
    arktest.assertEQ(str.indexOf('AaBaz'), CharNotFound);
    arktest.assertEQ(str.indexOf('aBab'), str.length - 'aBab'.length);
    arktest.assertEQ(str.indexOf('aBab', -1), str.length - 'aBab'.length);
    arktest.assertEQ(str.indexOf('aBab', 1), str.length - 'aBab'.length);
    arktest.assertEQ(str.indexOf('aBab', str.length - 'aBab'.length), str.length - 'aBab'.length);
    arktest.assertEQ(str.indexOf('aBab', str.length - 'AaBab'.length), str.length - 'aBab'.length);
    arktest.assertEQ(str.indexOf('aBab', str.length - 'Bab'.length), CharNotFound);
    arktest.assertEQ(str.indexOf('aBab', str.length - 1), CharNotFound);
    arktest.assertEQ(str.indexOf('aBab', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('aX'), CharNotFound);
    arktest.assertEQ(str.indexOf('!'), CharNotFound);
    arktest.assertEQ(str.indexOf('aXab'), CharNotFound);
    arktest.assertEQ(str.indexOf('a'), str.length - 'aBab'.length);
    arktest.assertEQ(str.indexOf('a', 1), str.length - 'aBab'.length);
    arktest.assertEQ(str.indexOf('a', -1), str.length - 'aBab'.length);
    arktest.assertEQ(str.indexOf('a', str.length - 1), CharNotFound);
    arktest.assertEQ(str.indexOf('a', str.length - 'ab'.length), str.length - 'ab'.length);
    arktest.assertEQ(str.indexOf('a', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('a', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('b', -1), str.length - 1);
    arktest.assertEQ(str.indexOf('b', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf('b', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('b', str.length + 1), CharNotFound);

    str = 'штзрЬоЯЯЙЫхAфыВы';
    arktest.assertEQ(str.indexOf('ш'), 0);
    arktest.assertEQ(str.indexOf('ш', -1), 0);
    arktest.assertEQ(str.indexOf('ш', -str.length), 0);
    arktest.assertEQ(str.indexOf('ш', 1), CharNotFound);
    arktest.assertEQ(str.indexOf('ш', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('ш', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('штзрЬоЯЯЙЫхAфыВыщ'), -1);
    arktest.assertEQ(str.indexOf('AфыВы'), str.length - 'АфыВы'.length);
    arktest.assertEQ(str.indexOf('AЙbВ'), CharNotFound);
    arktest.assertEQ(str.indexOf('AЙыВц'), CharNotFound);
    arktest.assertEQ(str.indexOf('Вц'), CharNotFound);
    arktest.assertEQ(str.indexOf('Ц'), CharNotFound);
    arktest.assertEQ(str.indexOf('AфыВ'), str.length - 'АфыВы'.length);
    arktest.assertEQ(str.indexOf('AфыВ', -1), str.length - 'АфыВы'.length);
    arktest.assertEQ(str.indexOf('AфыВ', str.length - 'АфыВы'.length), str.length - 'АфыВы'.length);
    arktest.assertEQ(str.indexOf('AфыВ', str.length - 'АфыВы'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('фыВ'), str.length - 'фыВы'.length);
    arktest.assertEQ(str.indexOf('фыВ', -1), str.length - 'фыВы'.length);
    arktest.assertEQ(str.indexOf('фыВ', 1), str.length - 'фыВы'.length);
    arktest.assertEQ(str.indexOf('фыВ', str.length - 'фыВы'.length), str.length - 'фыВы'.length);
    arktest.assertEQ(str.indexOf('фыВ', str.length - 'фыВы'.length - 1), str.length - 'фыВы'.length);
    arktest.assertEQ(str.indexOf('фыВ', str.length - 'фыВы'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('фыВ', str.length - 1), CharNotFound);
    arktest.assertEQ(str.indexOf('фыВ', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('ф'), str.length - 'фыВы'.length);
    arktest.assertEQ(str.indexOf('В'), str.length - 'Вы'.length);
    arktest.assertEQ(str.indexOf('В', str.length - 'Вы'.length), str.length - 'Вы'.length);
    arktest.assertEQ(str.indexOf('В', str.length - 'Вы'.length - 1), str.length - 'Вы'.length);
    arktest.assertEQ(str.indexOf('В', str.length - 'Вы'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('Вы'), str.length - 'Вы'.length);
    arktest.assertEQ(str.indexOf('Вы', str.length - 'Вы'.length), str.length - 'Вы'.length);
    arktest.assertEQ(str.indexOf('Бы'), CharNotFound);
    arktest.assertEQ(str.indexOf('Ab'), CharNotFound);
    arktest.assertEQ(str.indexOf('ы'), str.length - 'ыВы'.length);
    arktest.assertEQ(str.indexOf('ы', -1), str.length - 'ыВы'.length);
    arktest.assertEQ(str.indexOf('ы', str.length - 'фыВы'.length), str.length - 'ыВы'.length);
    arktest.assertEQ(str.indexOf('ы', str.length - 'ыВы'.length), str.length - 'ыВы'.length);
    arktest.assertEQ(str.indexOf('ы', str.length - 'Вы'.length), str.length - 1);
    arktest.assertEQ(str.indexOf('ы', str.length - 'ы'.length), str.length - 1);
    arktest.assertEQ(str.indexOf('ы', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('ы', str.length + 1), CharNotFound);

    str = 'штзрЬоЯЯЙЫхAфzWz';
    arktest.assertEQ(str.indexOf('ш'), 0);
    arktest.assertEQ(str.indexOf('ш', -1), 0);
    arktest.assertEQ(str.indexOf('ш', -str.length), 0);
    arktest.assertEQ(str.indexOf('ш', 1), CharNotFound);
    arktest.assertEQ(str.indexOf('ш', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('ш', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('штзрЬоЯЯЙЫхAфzWzщ'), -1);
    arktest.assertEQ(str.indexOf('zWz'), str.length - 'zWz'.length);
    arktest.assertEQ(str.indexOf('zWz', -1), str.length - 'zWz'.length);
    arktest.assertEQ(str.indexOf('zWz', str.length - 'zWz'.length), str.length - 'zWz'.length);
    arktest.assertEQ(str.indexOf('zWz', str.length - 'фzWz'.length), str.length - 'zWz'.length);
    arktest.assertEQ(str.indexOf('zWz', str.length - 'Wz'.length), CharNotFound);
    arktest.assertEQ(str.indexOf('zWx'), CharNotFound);
    arktest.assertEQ(str.indexOf('AфzWz'), str.length - 'АфzWz'.length);
    arktest.assertEQ(str.indexOf('AЙbW'), CharNotFound);
    arktest.assertEQ(str.indexOf('AЙфzWц'), CharNotFound);
    arktest.assertEQ(str.indexOf('хAфzWz'), str.length - 'хАфzWz'.length);
    arktest.assertEQ(str.indexOf('хAфxWz'), CharNotFound);
    arktest.assertEQ(str.indexOf('хAфzWx'), CharNotFound);
    arktest.assertEQ(str.indexOf('Wq'), CharNotFound);
    arktest.assertEQ(str.indexOf('Q'), CharNotFound);
    arktest.assertEQ(str.indexOf('AфzW'), str.length - 'АфzWz'.length);
    arktest.assertEQ(str.indexOf('AфzW', -1), str.length - 'АфzWz'.length);
    arktest.assertEQ(str.indexOf('AфzW', str.length - 'АфzWz'.length), str.length - 'АфzWz'.length);
    arktest.assertEQ(str.indexOf('AфzW', str.length - 'АфzWz'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('фzW'), str.length - 'фzWz'.length);
    arktest.assertEQ(str.indexOf('фzW', -1), str.length - 'фzWz'.length);
    arktest.assertEQ(str.indexOf('фzW', 1), str.length - 'фzWz'.length);
    arktest.assertEQ(str.indexOf('фzW', str.length - 'фzWz'.length), str.length - 'фzWz'.length);
    arktest.assertEQ(str.indexOf('фzW', str.length - 'фzWz'.length - 1), str.length - 'фzWz'.length);
    arktest.assertEQ(str.indexOf('фzW', str.length - 'фzWz'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('фzW', str.length - 1), CharNotFound);
    arktest.assertEQ(str.indexOf('фzW', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('ф'), str.length - 'фzWz'.length);
    arktest.assertEQ(str.indexOf('W'), str.length - 'Wz'.length);
    arktest.assertEQ(str.indexOf('W', str.length - 'Wz'.length), str.length - 'Wz'.length);
    arktest.assertEQ(str.indexOf('W', str.length - 'Wz'.length - 1), str.length - 'Wz'.length);
    arktest.assertEQ(str.indexOf('W', str.length - 'Wz'.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('Wz'), str.length - 'Wz'.length);
    arktest.assertEQ(str.indexOf('Wz', str.length - 'Wz'.length), str.length - 'Wz'.length);
    arktest.assertEQ(str.indexOf('Vz'), CharNotFound);
    arktest.assertEQ(str.indexOf('Ab'), CharNotFound);
    arktest.assertEQ(str.indexOf('z'), str.length - 'zWz'.length);
    arktest.assertEQ(str.indexOf('z', -1), str.length - 'zWz'.length);
    arktest.assertEQ(str.indexOf('z', str.length - 'фzWz'.length), str.length - 'zWz'.length);
    arktest.assertEQ(str.indexOf('z', str.length - 'zWz'.length), str.length - 'zWz'.length);
    arktest.assertEQ(str.indexOf('z', str.length - 'Wz'.length), str.length - 1);
    arktest.assertEQ(str.indexOf('z', str.length - 'z'.length), str.length - 1);
    arktest.assertEQ(str.indexOf('z', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('z', str.length + 1), CharNotFound);

    str = 'ЗрвяСуНопрЩAфzWl';
    arktest.assertEQ(str.indexOf('З'), 0);
    arktest.assertEQ(str.indexOf('З', -1), 0);
    arktest.assertEQ(str.indexOf('З', -str.length), 0);
    arktest.assertEQ(str.indexOf('З', 1), CharNotFound);
    arktest.assertEQ(str.indexOf('З', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('З', str.length + 1), CharNotFound);
    arktest.assertEQ(str.indexOf('ЗрвяСуНопрЩAфzWlч'), -1);
    arktest.assertEQ(str.indexOf('AфzWl'), str.length - 'АфzWl'.length);
    arktest.assertEQ(str.indexOf('AЙzWm'), CharNotFound);
    arktest.assertEQ(str.indexOf('AфzWm'), CharNotFound);
    arktest.assertEQ(str.indexOf('AzWl'), CharNotFound);
    arktest.assertEQ(str.indexOf('zWn'), CharNotFound);
    arktest.assertEQ(str.indexOf('zWl'), str.length - 'zWl'.length);
    arktest.assertEQ(str.indexOf('zWl', -1), str.length - 'zWl'.length);
    arktest.assertEQ(str.indexOf('zWl', str.length - 'zWl'.length), str.length - 'zWl'.length);
    arktest.assertEQ(str.indexOf('zWl', str.length - 'фzWl'.length), str.length - 'zWl'.length);
    arktest.assertEQ(str.indexOf('zWl', str.length - 'Wl'.length), CharNotFound);
    arktest.assertEQ(str.indexOf('W'), str.length - 'Wl'.length);
    arktest.assertEQ(str.indexOf('W', str.length - 'Wl'.length), str.length - 'Wl'.length);
    arktest.assertEQ(str.indexOf('W', str.length - 'zWl'.length), str.length - 'Wl'.length);
    arktest.assertEQ(str.indexOf('W', str.length - 'l'.length), CharNotFound);
    arktest.assertEQ(str.indexOf('Wl'), str.length - 'Wl'.length);
    arktest.assertEQ(str.indexOf('Wl', str.length - 'Wl'.length), str.length - 'Wl'.length);
    arktest.assertEQ(str.indexOf('Wm'), CharNotFound);
    arktest.assertEQ(str.indexOf('l'), str.length - 1);
    arktest.assertEQ(str.indexOf('l', -1), str.length - 1);
    arktest.assertEQ(str.indexOf('l', str.length - 2), str.length - 1);
    arktest.assertEQ(str.indexOf('l', str.length - 1), str.length - 1);
    arktest.assertEQ(str.indexOf('l', str.length), CharNotFound);
    arktest.assertEQ(str.indexOf('l', str.length + 1), CharNotFound);
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
    let mixedTreeChars = EscChars.concat(RusChars);
    testAllSubstrings(mixedTreeChars);
}

function testEscapedSlicedSubstring() {
    let escSlicedChars = EscChars.substring(1, EscChars.length - 2);
    testAllSubstrings(escSlicedChars);
}

function testNonCompressedLinearString() {
    arktest.assertEQ(RusChars.length, 66);
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
    arktest.assertEQ(rusTreeChars.indexOf(rusChars_1 + '!'), CharNotFound);
    arktest.assertEQ(rusTreeChars.indexOf(rusChars_1.concat('!')), CharNotFound);
    arktest.assertEQ(rusTreeChars.indexOf(rusChars_2 + '!'), CharNotFound);
    arktest.assertEQ(rusTreeChars.indexOf(rusChars_2.concat('!')), CharNotFound);
    arktest.assertEQ(RusChars.indexOf(rusTreeChars, 1), CharNotFound);

    let rusTreeChars2 = rusTreeChars.concat('!');
    arktest.assertEQ(rusTreeChars2.indexOf(rusChars_1), 0);
    arktest.assertEQ(rusTreeChars2.indexOf(rusChars_2), rusChars_1.length);
    arktest.assertEQ(rusTreeChars2.indexOf(rusChars_2, rusChars_1.length), rusChars_1.length);
    arktest.assertEQ(RusChars.indexOf(rusTreeChars2, 0), CharNotFound);
    arktest.assertEQ(rusTreeChars2.indexOf(rusChars_1 + '!'), CharNotFound);
    arktest.assertEQ(rusTreeChars2.indexOf(rusChars_1.concat('!')), CharNotFound);
    arktest.assertEQ(rusTreeChars2.indexOf(rusChars_2 + '!'), rusChars_1.length);
    arktest.assertEQ(rusTreeChars2.indexOf(rusChars_2.concat('!')), rusChars_1.length);
    arktest.assertEQ(rusTreeChars2.indexOf(rusTreeChars2.concat('!')), CharNotFound);
    arktest.assertEQ(rusTreeChars2.indexOf(rusTreeChars2 + '!'), CharNotFound);
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

    let rusTreeCharsTooLongKey = rusChars_1.concat(rusChars_2).concat('X');
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
    arktest.assertEQ(EngChars.length, 52);
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
    arktest.assertEQ(engTreeChars.indexOf(engChars_1 + '!'), CharNotFound);
    arktest.assertEQ(engTreeChars.indexOf(engChars_1.concat('!')), CharNotFound);
    arktest.assertEQ(engTreeChars.indexOf(engChars_2 + '!'), CharNotFound);
    arktest.assertEQ(engTreeChars.indexOf(engChars_2.concat('!')), CharNotFound);
    arktest.assertEQ(engTreeChars.indexOf(engTreeChars.concat('R')), CharNotFound);
    arktest.assertEQ(engTreeChars.indexOf(engTreeChars + 'R'), CharNotFound);
    arktest.assertEQ(engTreeChars.indexOf('R'.concat(engTreeChars)), CharNotFound);
    arktest.assertEQ(engTreeChars.indexOf('R' + engTreeChars), CharNotFound);

    let engTreeChars2 = engTreeChars.concat('!');
    arktest.assertEQ(engTreeChars2.indexOf(engChars_1), 0);
    arktest.assertEQ(engTreeChars2.indexOf(engChars_2), engChars_1.length);
    arktest.assertEQ(engTreeChars2.indexOf(engChars_1 + '!'), CharNotFound);
    arktest.assertEQ(engTreeChars2.indexOf(engChars_1.concat('!')), CharNotFound);
    arktest.assertEQ(engTreeChars2.indexOf(engChars_2 + '!'), engChars_1.length);
    arktest.assertEQ(engTreeChars2.indexOf(engChars_2.concat('!')), engChars_1.length);
    arktest.assertEQ(engTreeChars2.indexOf(engTreeChars2.concat('R')), CharNotFound);
    arktest.assertEQ(engTreeChars2.indexOf(engTreeChars2 + 'R'), CharNotFound);
    arktest.assertEQ(engTreeChars2.indexOf('R'.concat(engTreeChars2)), CharNotFound);
    arktest.assertEQ(engTreeChars2.indexOf('R' + engTreeChars2), CharNotFound);
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

    let engTreeCharsTooLongKey = engChars_1.concat(engChars_2).concat('X');
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

    let mixedTreeChars2 = mixedTreeChars.concat('!');
    arktest.assertEQ(mixedTreeChars2.indexOf(rusChars, 1), CharNotFound);
    arktest.assertEQ(mixedTreeChars2.indexOf(rusChars + '!'), CharNotFound);
    arktest.assertEQ(mixedTreeChars2.indexOf(rusChars.concat('!')), CharNotFound);
    arktest.assertEQ(mixedTreeChars2.indexOf(engChars + '!'), rusChars.length);
    arktest.assertEQ(mixedTreeChars2.indexOf(engChars.concat('!')), rusChars.length);
    arktest.assertEQ(mixedTreeChars2.indexOf(mixedTreeChars2 + '!'), CharNotFound);
    arktest.assertEQ(mixedTreeChars2.indexOf(mixedTreeChars2.concat('!')), CharNotFound);
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

    let mixedSlicedTooLongKey = (rusCharsKey.concat(engCharsKey).concat('!!!')).substring(1,
        rusCharsKey.length + engCharsKey.length + 2);
    arktest.assertLT(mixedSlicedChars.length, mixedSlicedTooLongKey.length);
    arktest.assertEQ(mixedSlicedChars.indexOf(mixedSlicedTooLongKey), CharNotFound);
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
    let mixedSlicedChars = mixedTreeChars.substring(3, rusChars.length - 4);
    testAllSubstrings(mixedSlicedChars);
    mixedSlicedChars = mixedTreeChars.substring(3, rusChars.length + 15);
    testAllSubstrings(mixedSlicedChars);
    arktest.assertLE(20, engChars.length);
    mixedSlicedChars = mixedTreeChars.substring(4, rusChars.length + 20);
    testAllSubstrings(mixedSlicedChars);
    arktest.assertLE(18, engChars.length);
    mixedSlicedChars = mixedTreeChars.substring(rusChars.length + 1, rusChars.length + 18);
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

function testVeryLongMixedCompressingLinearString() {
    let longMixedChars: String = 'АаБбВвГгДдЕеЁёЖжAaBbCcDdEeFfGgHhIiJjKkLlнфUghДвгатфLpfdjdOklbZodissboЗзИиЙйКкЛлМмНнrofjweflkwlщцшуддцйзжцйэйцюцйффыогцуоцуюдйцжйжцхэpwpqwKJisisiYIUЮКОДвгатфLpfdjdOklbZodissbйцьйцйт';
    arktest.assertEQ(180, longMixedChars.length)
    let rusChars = 'АаБбВвГгДдЕеЁёЖж';
    let engChars = 'AaBbCcDdEeFfGgH';
    let smallPrefix = 'АаБб';
    arktest.assertEQ(longMixedChars.indexOf(longMixedChars), 0);
    arktest.assertEQ(longMixedChars.indexOf(longMixedChars, 1), CharNotFound);
    arktest.assertEQ(longMixedChars.indexOf(smallPrefix), 0);
    arktest.assertEQ(longMixedChars.indexOf(smallPrefix, 1), CharNotFound);
    arktest.assertEQ(longMixedChars.indexOf(rusChars), 0);
    arktest.assertEQ(longMixedChars.indexOf(engChars), rusChars.length);

    let longMixedCharsPlus: String = 'АаБбВвГгДдЕеЁёЖжAaBbCcDdEeFfGgHhIiJjKkLlнфUghДвгатфLpfdjdOklbZodissboЗзИиЙйКкЛлМмНнrofjweflkwlщцшуддцйзжцйэйцюцйффыогцуоцуюдйцжйжцхэpwpqwKJisisiYIUЮКОДвгатфLpfdjdOklbZodissbйцьйцйт!';
    arktest.assertLT(180, longMixedCharsPlus.length)
    arktest.assertEQ(longMixedCharsPlus.indexOf(longMixedChars), 0);
    arktest.assertEQ(longMixedCharsPlus.indexOf(longMixedChars), 0);
    arktest.assertEQ(longMixedCharsPlus.indexOf(longMixedChars, 1), CharNotFound);
    arktest.assertEQ(longMixedCharsPlus.indexOf(longMixedChars, 1), CharNotFound);
    arktest.assertEQ(longMixedCharsPlus.indexOf(smallPrefix), 0);
    arktest.assertEQ(longMixedCharsPlus.indexOf(smallPrefix, 1), CharNotFound);
    arktest.assertEQ(longMixedCharsPlus.indexOf(rusChars), 0);
    arktest.assertEQ(longMixedCharsPlus.indexOf(engChars), rusChars.length);
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
