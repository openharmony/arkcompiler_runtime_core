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
const Wss: String =
'\u0020\u0009\u000A\u000B\u000C\u000D\u00A0\u1680\
\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\
\u2008\u2009\u200A\u2028\u2029\u202F\u205F\uFEFF\u3000';

const Empty: String = '';
const Ws: String = ' ';
const X: String = 'x';
const XWs: String = X + Ws;
const WsX: String = Ws + X;
const WsXWs: String = Ws + X + Ws;
const Xy: String = 'Xy';
const WsXy: String = Ws + 'Xy';
const XyWs: String = 'Xy' + Ws;
const WsXyWs: String = Ws + 'Xy' + Ws;
const Z: String = '\u01B5';  // 'Ƶ'
const ZWs: String = Z + Ws;
const WsZ: String = Ws + Z;
const WsZWs: String = Ws + Z + Ws;
const Abc: String = 'abc';
const AwsBwsC: String = 'a b c';
const WsAbc: String = ' ' + Abc;
const AbcWs: String = Abc + ' ';
const WsAbcWs: String = ' ' + Abc + ' ';
const WssAbc: String = Wss + Abc;
const AbcWss: String = Abc + Wss;
const WssAbcWss: String = Wss + Abc + Wss;
const AbcWssAbc: String = Abc + Wss + Abc;
const Esc: String = ' \t\n\v';
const RusChars: String = 'Русские буквы';

function main(): int {

    let wssCount: int = 0;
    for (let i = 0; i <= 0xFFFF; ++i) {
        let c = i as char;
        if (Char.isWhiteSpace(c)) {
            ++wssCount;
            assertNE(Wss.indexOf(c), -1, 'Bad char code is encountered: ' + i);
        }
    }
    assertEQ(wssCount, Wss.length, 'Wrong number of u16 whitespaces: ' + wssCount);

    // trim
    assertEQ(Empty.trim(), "")
    assertEQ(Ws.trim(), Empty)
    assertEQ(Wss.trim(), Empty)
    assertEQ(Xy.trim(), Xy)
    assertEQ(WsXy.trim(), Xy)
    assertEQ(XyWs.trim(), Xy)
    assertEQ(WsXyWs.trim(), Xy)
    assertEQ(Abc.trim(), Abc)
    assertEQ(WsAbc.trim(), Abc)
    assertEQ(AbcWs.trim(), Abc)
    assertEQ(WsAbcWs.trim(), Abc)
    assertEQ(AwsBwsC.trim(), AwsBwsC)
    assertEQ(WssAbc.trim(), Abc)
    assertEQ(AbcWss.trim(), Abc)
    assertEQ(WssAbcWss.trim(), Abc)
    assertEQ(AbcWssAbc.trim(), AbcWssAbc)
    assertEQ(X.trim(), X)
    assertEQ(WsX.trim(), X)
    assertEQ(XWs.trim(), X)
    assertEQ(Z.trim(), Z)
    assertEQ(WsZ.trim(), Z)
    assertEQ(ZWs.trim(), Z)
    assertEQ(Esc.trim(), Empty)
    assertEQ(RusChars.trim(), RusChars)
    // trimLeft
    assertEQ(Ws.trimLeft(), Empty)
    assertEQ(Wss.trimLeft(), Empty)
    assertEQ(WsAbcWs.trimLeft(), AbcWs)
    assertEQ(AwsBwsC.trimLeft(), AwsBwsC)
    assertEQ(WssAbcWss.trimLeft(), AbcWss)
    assertEQ(X.trimLeft(), X)
    assertEQ(WsX.trimLeft(), X)
    assertEQ(XWs.trimLeft(), XWs)
    assertEQ(Z.trimLeft(), Z)
    assertEQ(WsZ.trimLeft(), Z)
    assertEQ(ZWs.trimLeft(), ZWs)
    assertEQ(Esc.trimLeft(), Empty)
    assertEQ(RusChars.trimLeft(), RusChars)
    // trimRight
    assertEQ(Ws.trimRight(), Empty)
    assertEQ(Wss.trimRight(), Empty)
    assertEQ(WsAbcWs.trimRight(), WsAbc)
    assertEQ(AwsBwsC.trimRight(), AwsBwsC)
    assertEQ(WssAbcWss.trimRight(), WssAbc)
    assertEQ(X.trimRight(), X)
    assertEQ(WsX.trimRight(), WsX)
    assertEQ(XWs.trimRight(), X)
    assertEQ(Z.trimRight(), Z)
    assertEQ(WsZ.trimRight(), WsZ)
    assertEQ(ZWs.trimRight(), Z)
    assertEQ(Esc.trimRight(), Empty)
    assertEQ(RusChars.trimRight(), RusChars)

    return 0;
}
