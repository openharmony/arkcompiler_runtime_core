/**
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

const emptyString = '';
const oneElementArrayOfEmptyString = [(emptyString)];
const emptyArrayOfStrings: String[] = [];

let s0 = 'abc';
let sa0 = ['a', 'b', 'c'];
let sa01 = ['a'];
let sa02 = ['a', 'b'];

let s1 = 'abc+def+ghi';
let sa1 = ['abc', 'def', 'ghi'];
let sa11 = ['abc'];
let sa12 = ['abc', 'def'];

let s2 = '+abc+def+ghi+';
let sa2 = ['', 'abc', 'def', 'ghi', ''];
let sa21 = [''];
let sa25 = ['', 'abc', 'def', 'ghi', ''];

let s3 = 'aSEPbSEPcSEPdSEPeSEPfSEPgSEPhSEPiSEPjSEPkSEPlSEPmSEPnSEPpSEPqSEPrSEPsSEPtSEPuSEPvSEPwSEPxSEPySEPzSEP';
let sa3 = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', ''];
let sa31 = ['a'];

let s4 = 'aQwerty1234567890bQwerty1234567890cQwerty1234567890dQwerty1234567890eQwerty1234567890fQwerty1234567890gQwerty1234567890\
hQwerty1234567890iQwerty1234567890jQwerty1234567890kQwerty1234567890lQwerty1234567890mQwerty1234567890nQwerty1234567890pQwerty1234567890\
qQwerty1234567890rQwerty1234567890sQwerty1234567890tQwerty1234567890uQwerty1234567890vQwerty1234567890wQwerty1234567890xQwerty1234567890\
yQwerty1234567890zQwerty1234567890';
let sa4 = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', ''];
let sa41 = ['a'];

let ws1 = '一二三四一二三四一二三四一二三四一二三四一二三';
let wsa1 = ['一二三', '一二三', '一二三', '一二三', '一二三', '一二三'];
let wsa11 = ['一二三'];

let ws2 = '一二三四一二三四一二三四一二三四一二三四一二三四';
let wsa2 = ['一二三', '一二三', '一二三', '一二三', '一二三', '一二三', ''];
let wsa22 = ['一二三'];

let ws3 = 'аРаРыРуРяРиРюбРаРыРуРяРиРювРаРыРуРяРиРюгРаРыРуРяРиРюдРаРыРуРяРиРюеРаРыРуРяРиРюжРаРыРуРяРиРю\
зРаРыРуРяРиРюиРаРыРуРяРиРюйРаРыРуРяРиРюкРаРыРуРяРиРюлРаРыРуРяРиРюмРаРыРуРяРиРюнРаРыРуРяРиРюоРаРыРуРяРиРю\
пРаРыРуРяРиРюрРаРыРуРяРиРюсРаРыРуРяРиРют';
let wsa3 = ['а', 'б', 'в', 'г', 'д', 'е', 'ж', 'з', 'и', 'й', 'к', 'л', 'м', 'н', 'о', 'п', 'р', 'с', 'т'];
let wsa31 = ['а'];

let ws4 = 'аабрикосаананасаарбуз';
let wsa4 = ['', '', 'брикос', '', 'н', 'н', 'с', '', 'рбуз'];
let wsa41 = [''];

// RegEx related
type StringOrRegExp = String | RegExp;
let rxs0 = 'What is in	a name?';
let rxa0 = ['What', 'is', 'in', 'a', 'name?'];
let rxs1 = 'What1is22in333a4444name55555?';
let rxa1 = ['What', 'is', 'in', 'a', 'name', '?'];
let rxs2 = 'How1are1you1doing?';
let rxa2 = ['How', 'are', 'you', 'doing?'];

// String containing surrogate pairs
const sur1 = '𝟘𝟙𝟚𝟛';
const sur1Pairs: Int[] = [0xd835, 0xdfd8, 0xd835, 0xdfd9, 0xd835, 0xdfda, 0xd835, 0xdfdb];
const sur2 = '𝟘𝟙𝟚𝟛𝟘𝟙𝟚𝟛𝟘𝟙𝟚𝟛𝟘𝟙𝟚𝟛';
const sur3 = '𝟘𝟙𝟚𝟛😄𝟘𝟙𝟚𝟛😄𝟘𝟙𝟚𝟛😄𝟘𝟙𝟚𝟛';
const sur4 = '😄𝟘𝟙𝟚𝟛😄𝟘𝟙𝟚𝟛😄𝟘𝟙𝟚𝟛😄𝟘𝟙𝟚𝟛😄';
const sur5 = '🐧🐼🐻🦋🦊🐸🦉🦋🐘🐮🐶';
const sur6 = 'abc🦋def🦋ghi';
const sur7 = 'абв🦋где🦋жзи';

function cmpArr(lhs: Array<String>, rhs: Array<String>): boolean {
    if (lhs.length != rhs.length) {
        return false;
    }
    for (let i = 0; i < lhs.length; ++i) {
        if (!lhs[i].equals(rhs[i])) {
            return false;
        }
    }
    return true;
}


function main() {

    arktest.assertTrue(cmpArr(emptyString.split(''), emptyArrayOfStrings));
    arktest.assertTrue(cmpArr(emptyString.split('dummy'), oneElementArrayOfEmptyString));

    arktest.assertTrue(cmpArr(s0.split(''), sa0));
    arktest.assertTrue(cmpArr(s0.split('', -100), sa0));
    arktest.assertTrue(cmpArr(s0.split('', 0), emptyArrayOfStrings));
    arktest.assertTrue(cmpArr(s0.split('', 1), sa01));
    arktest.assertTrue(cmpArr(s0.split('', 2), sa02));
    arktest.assertTrue(cmpArr(s0.split('', 3), sa0));

    arktest.assertTrue(cmpArr(s1.split('+'), sa1));
    arktest.assertTrue(cmpArr(s1.split('+', 1), sa11));
    arktest.assertTrue(cmpArr(s1.split('+', 2), sa12));

    arktest.assertTrue(cmpArr(s2.split('+'), sa2));
    arktest.assertTrue(cmpArr(s2.split('+', 1), sa21));
    arktest.assertTrue(cmpArr(s2.split('+', 5), sa25));
    arktest.assertTrue(cmpArr(s2.split('+', 6), sa25));

    arktest.assertTrue(cmpArr(s3.split('SEP'), sa3));
    arktest.assertTrue(cmpArr(s3.split('', 0), emptyArrayOfStrings));
    arktest.assertTrue(cmpArr(s3.split('SEP', -1), sa3));
    arktest.assertTrue(cmpArr(s3.split('SEP', Int.MAX_VALUE), sa3));
    arktest.assertTrue(cmpArr(s3.split('SEP', 1), sa31));

    arktest.assertTrue(cmpArr(s4.split('Qwerty1234567890'), sa4));
    arktest.assertTrue(cmpArr(s4.split('', 0), emptyArrayOfStrings));
    arktest.assertTrue(cmpArr(s4.split('Qwerty1234567890', 10000000), sa4));
    arktest.assertTrue(cmpArr(s4.split('Qwerty1234567890', -1), sa4));
    arktest.assertTrue(cmpArr(s4.split('Qwerty1234567890', Int.MAX_VALUE), sa4));
    arktest.assertTrue(cmpArr(s4.split('Qwerty1234567890', 1), sa41));

    arktest.assertTrue(cmpArr(ws1.split('四'), wsa1));
    arktest.assertTrue(cmpArr(ws1.split('四', -1), wsa1));
    arktest.assertTrue(cmpArr(ws1.split('四', 0), emptyArrayOfStrings));
    arktest.assertTrue(cmpArr(ws1.split('四', 1), wsa11));
    arktest.assertTrue(cmpArr(ws1.split('四', 100), wsa1));

    arktest.assertTrue(cmpArr(ws2.split('四'), wsa2));
    arktest.assertTrue(cmpArr(ws2.split('四', -1), wsa2));
    arktest.assertTrue(cmpArr(ws2.split('四', 0), emptyArrayOfStrings));
    arktest.assertTrue(cmpArr(ws2.split('四', 1), wsa22));
    arktest.assertTrue(cmpArr(ws2.split('四', 100), wsa2));

    arktest.assertTrue(cmpArr(ws3.split('РаРыРуРяРиРю'), wsa3));
    arktest.assertTrue(cmpArr(ws3.split('', 0), emptyArrayOfStrings));
    arktest.assertTrue(cmpArr(ws3.split('РаРыРуРяРиРю', 10000000), wsa3));
    arktest.assertTrue(cmpArr(ws3.split('РаРыРуРяРиРю', -1), wsa3));
    arktest.assertTrue(cmpArr(ws3.split('РаРыРуРяРиРю', Int.MAX_VALUE), wsa3));
    arktest.assertTrue(cmpArr(ws3.split('РаРыРуРяРиРю', 1), wsa31));

    arktest.assertTrue(cmpArr(ws4.split('а'), wsa4));
    arktest.assertTrue(cmpArr(ws4.split('', 0), emptyArrayOfStrings));
    arktest.assertTrue(cmpArr(ws4.split('а', 10000000), wsa4));
    arktest.assertTrue(cmpArr(ws4.split('а', -1), wsa4));
    arktest.assertTrue(cmpArr(ws4.split('а', Int.MAX_VALUE), wsa4));
    arktest.assertTrue(cmpArr(ws4.split('а', 1), wsa41));

    // Some string or regex checks
    let sr1: StringOrRegExp = new RegExp('\\s+');
    let sr2: StringOrRegExp = new RegExp('[0-9]+');
    let sr3: StringOrRegExp = String('1');
    arktest.assertTrue(cmpArr(rxs0.split(sr1), rxa0));
    arktest.assertTrue(cmpArr(rxs1.split(sr2), rxa1));
    arktest.assertTrue(cmpArr(rxs2.split(sr3), rxa2));

    /* Check strings containing surrogate pairs.
     *
     * NOTE, If ('') is used as a separator, the string is split by utf-16 code units,
     * and this destroys surrogate pairs.
     */
    let a1 = '𝟘𝟙𝟚𝟛'.split('');
    arktest.assertEQ(a1.length, 8);
    arktest.assertEQ(a1.length, sur1Pairs.length);
    for (let i = 0; i < a1.length; ++i) {
        let s = a1.at(i);
        for (let j = 0; j < s.length; ++j) {
            arktest.assertEQ(s.codePointAt(j)!, sur1Pairs.at(i + j));
        }
    }
    let a2 = sur1.split('𝟚');
    arktest.assertTrue(cmpArr(a2, ['𝟘𝟙', '𝟛']));
    let a3 = sur2.split('𝟘');
    arktest.assertTrue(cmpArr(a3, ['', '𝟙𝟚𝟛', '𝟙𝟚𝟛', '𝟙𝟚𝟛', '𝟙𝟚𝟛']));
    let a4 = sur2.split('𝟙𝟚𝟛');
    arktest.assertTrue(cmpArr(a4, ['𝟘', '𝟘', '𝟘', '𝟘', '']));
    let a5 = sur3.split('😄');
    arktest.assertTrue(cmpArr(a5, ['𝟘𝟙𝟚𝟛', '𝟘𝟙𝟚𝟛', '𝟘𝟙𝟚𝟛', '𝟘𝟙𝟚𝟛']));
    let a6 = sur4.split('😄');
    arktest.assertTrue(cmpArr(a6, ['', '𝟘𝟙𝟚𝟛', '𝟘𝟙𝟚𝟛', '𝟘𝟙𝟚𝟛', '𝟘𝟙𝟚𝟛', '']));
    let a7 = sur5.split('🦋');
    arktest.assertTrue(cmpArr(a7, ['🐧🐼🐻', '🦊🐸🦉', '🐘🐮🐶']));
    let a8 = sur6.split('🦋');
    arktest.assertTrue(cmpArr(a8, ['abc', 'def', 'ghi']));
    let a9 = sur7.split('🦋');
    arktest.assertTrue(cmpArr(a9, ['абв', 'где', 'жзи']));
}
