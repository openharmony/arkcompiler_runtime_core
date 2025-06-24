/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

/* White space charcodes:
    0x0020 -- space
    0x0009 -- horizontal tab
    0x000A -- line feed or new line
    0x000B -- vertical tab
    0x000C -- formfeed
    0x000D -- horizontal tab
    0x00A0 -- no-break space
    0x1680 -- Ogham space mark
    0x2000 -- en quad
    0x2001 -- em quad
    0x2002 -- en space
    0x2003 -- em space
    0x2004 -- three-per-em space
    0x2005 -- four-per-em space
    0x2006 -- six-per-em space
    0x2007 -- figure space
    0x2008 -- punctuation space
    0x2009 -- thin space
    0x200A -- hair space
    0x2028 -- line separator
    0x2029 -- paragraph separator
    0x202F -- narrow no-break space
    0x205F -- medium mathematical space
    0xFEFF -- byte order mark
    0x3000 -- ideographic space
*/
const gWss: String =
'\u0020\u0009\u000A\u000B\u000C\u000D\u00A0\u1680\
\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\
\u2008\u2009\u200A\u2028\u2029\u202F\u205F\uFEFF\u3000';

const gEmpty: String = '';
const gWs: String = ' ';
const gX: String = 'x';
const gXWs: String = gX + gWs;
const gWsX: String = gWs + gX;
const gWsXWs: String = gWs + gX + gWs;
const gXy: String = 'gXy';
const gWsXy: String = gWs + 'gXy';
const gXyWs: String = 'gXy' + gWs;
const gWsXyWs: String = gWs + 'gXy' + gWs;
const gZ: String = '\u01B5'; // 'Ƶ'
const gZWs: String = gZ + gWs;
const gWsZ: String = gWs + gZ;
const gWsZWs: String = gWs + gZ + gWs;
const gAbc: String = 'abc';
const gAwsBwsC: String = 'a b c';
const gWsAbc: String = ' ' + gAbc;
const gAbcWs: String = gAbc + ' ';
const gWsAbcWs: String = ' ' + gAbc + ' ';
const gWssAbc: String = gWss + gAbc;
const gAbcWss: String = gAbc + gWss;
const gWssAbcWss: String = gWss + gAbc + gWss;
const gAbcWssAbc: String = gAbc + gWss + gAbc;
const gEsc: String = ' \t\n\v';
const gRusChars: String = 'Русские буквы';

function main(): int {

    let wssCount: int = 0;
    for (let i = 0; i <= 0xFFFF; ++i) {
        let c = i as char;
        if (Char.isWhiteSpace(c)) {
            ++wssCount;
            arktest.assertNE(gWss.indexOf(c), -1, 'Bad char code is encountered: ' + i);
        }
    }
    arktest.assertEQ(wssCount, gWss.length, 'Wrong number of u16 whitespaces: ' + wssCount);

    // trim
    arktest.assertEQ(gEmpty.trim(), '')
    arktest.assertEQ(gWs.trim(), gEmpty)
    arktest.assertEQ(gWss.trim(), gEmpty)
    arktest.assertEQ(gXy.trim(), gXy)
    arktest.assertEQ(gWsXy.trim(), gXy)
    arktest.assertEQ(gXyWs.trim(), gXy)
    arktest.assertEQ(gWsXyWs.trim(), gXy)
    arktest.assertEQ(gAbc.trim(), gAbc)
    arktest.assertEQ(gWsAbc.trim(), gAbc)
    arktest.assertEQ(gAbcWs.trim(), gAbc)
    arktest.assertEQ(gWsAbcWs.trim(), gAbc)
    arktest.assertEQ(gAwsBwsC.trim(), gAwsBwsC)
    arktest.assertEQ(gWssAbc.trim(), gAbc)
    arktest.assertEQ(gAbcWss.trim(), gAbc)
    arktest.assertEQ(gWssAbcWss.trim(), gAbc)
    arktest.assertEQ(gAbcWssAbc.trim(), gAbcWssAbc)
    arktest.assertEQ(gX.trim(), gX)
    arktest.assertEQ(gWsX.trim(), gX)
    arktest.assertEQ(gXWs.trim(), gX)
    arktest.assertEQ(gZ.trim(), gZ)
    arktest.assertEQ(gWsZ.trim(), gZ)
    arktest.assertEQ(gZWs.trim(), gZ)
    arktest.assertEQ(gEsc.trim(), gEmpty)
    arktest.assertEQ(gRusChars.trim(), gRusChars)
    // trimLeft
    arktest.assertEQ(gWs.trimLeft(), gEmpty)
    arktest.assertEQ(gWss.trimLeft(), gEmpty)
    arktest.assertEQ(gWsAbcWs.trimLeft(), gAbcWs)
    arktest.assertEQ(gAwsBwsC.trimLeft(), gAwsBwsC)
    arktest.assertEQ(gWssAbcWss.trimLeft(), gAbcWss)
    arktest.assertEQ(gX.trimLeft(), gX)
    arktest.assertEQ(gWsX.trimLeft(), gX)
    arktest.assertEQ(gXWs.trimLeft(), gXWs)
    arktest.assertEQ(gZ.trimLeft(), gZ)
    arktest.assertEQ(gWsZ.trimLeft(), gZ)
    arktest.assertEQ(gZWs.trimLeft(), gZWs)
    arktest.assertEQ(gEsc.trimLeft(), gEmpty)
    arktest.assertEQ(gRusChars.trimLeft(), gRusChars)
    // trimRight
    arktest.assertEQ(gWs.trimRight(), gEmpty)
    arktest.assertEQ(gWss.trimRight(), gEmpty)
    arktest.assertEQ(gWsAbcWs.trimRight(), gWsAbc)
    arktest.assertEQ(gAwsBwsC.trimRight(), gAwsBwsC)
    arktest.assertEQ(gWssAbcWss.trimRight(), gWssAbc)
    arktest.assertEQ(gX.trimRight(), gX)
    arktest.assertEQ(gWsX.trimRight(), gWsX)
    arktest.assertEQ(gXWs.trimRight(), gX)
    arktest.assertEQ(gZ.trimRight(), gZ)
    arktest.assertEQ(gWsZ.trimRight(), gWsZ)
    arktest.assertEQ(gZWs.trimRight(), gZ)
    arktest.assertEQ(gEsc.trimRight(), gEmpty)
    arktest.assertEQ(gRusChars.trimRight(), gRusChars)

    return 0;
}
