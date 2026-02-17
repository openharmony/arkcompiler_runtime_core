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

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

//! CHECKER       AOT IR Builder, check StringBuilder internal buffer reserve
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*Reserve[0-9]*' --compiler-inlining=true"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::tryCatchReserve30"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::tryFinallyReserve31"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=2)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::outReserve32"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::inReserve33"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::globalReserve34"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::chainReserve35"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=2)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::moreTypesReserve36"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=20)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=20)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::nestedLoopReserve37"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=12)"
//! INST_NOT      "NewArray (size=9)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=12)"
//! INST          "NewArray (size=9)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::storeReserve39"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::chainReserve40"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::chainReserve41"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::argReserve42"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::storeReserve43"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::storeReserve44"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"

//! CHECKER       JIT IR Builder, check StringBuilder internal buffer reserve
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true,
//!               options: "--compiler-regex='.*Reserve[0-9]*' --compiler-inlining=true",
//!               entry: "ets_string_builder_reserve_part30-44.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::tryCatchReserve30"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::tryFinallyReserve31"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=2)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::outReserve32"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::inReserve33"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::globalReserve34"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::chainReserve35"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=2)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::moreTypesReserve36"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=20)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=20)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::nestedLoopReserve37"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=12)"
//! INST_NOT      "NewArray (size=9)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=12)"
//! INST          "NewArray (size=9)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::storeReserve39"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::chainReserve40"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::chainReserve41"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::argReserve42"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::storeReserve43"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::storeReserve44"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"

//! CHECKER       JIT IR Builder, check StringBuilder internal buffer reallocation events occured
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true,
//!               options: "--compiler-regex='.*main.*' --compiler-reserve-string-builder-buffer=false \
//!                         --compiler-inlining=true",
//!               entry: "ets_string_builder_reserve_part30-44.ETSGLOBAL::mainEventsCheck"
//! EVENT         "SbBufferRealloc"

//! CHECKER       JIT IR Builder, check StringBuilder internal buffer reallocation events not occured
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true,
//!               options: "--compiler-regex='.*main.*' --compiler-reserve-string-builder-buffer=true \
//!                         --compiler-inlining=true",
//!               entry: "ets_string_builder_reserve_part30-44.ETSGLOBAL::mainEventsCheck"
//! EVENT_NOT     "SbBufferRealloc"

function tryCatchReserve30(doThrow: boolean) : string {
    let sb = new StringBuilder()            // applied
    try {
        sb.append(0)
        if (doThrow) {
            throw new Error('Error')
        }
    }
    catch (ex) {
        sb.append('1')
        sb.append(ex)
    }
    return sb.toString()
}

function tryFinallyReserve31() : string {
    let sb = new StringBuilder()            // applied
    try {
        sb.append(0)
    }
    finally {
        sb.append('1')
    }
    return sb.toString()
}

function outReserve32() : StringBuilder {
    let sb = new StringBuilder()            // not applied, due to object being stored/returned
    sb.append(0)
    return sb
}

function inReserve33(sb : StringBuilder) : String {
    sb.append(1)                            // not applied, due to instance allocated outside block
    return sb.toString()
}

let sbGlobal : StringBuilder = new StringBuilder
function globalReserve34() : String {
    sbGlobal.append(0)                            // not applied, due to instance allocated outside block
    return sbGlobal.toString()
}

function chainReserve35() : String {
    return new StringBuilder().append(0).append(1).toString()   // applied
}

function moreTypesReserve36() : String {
    const emptyString = "";
    let nullString: String = 'null';
    let chArray: FixedArray<char> = [c'!', c'@', c'#', c'$', c'%'];

    let sb: StringBuilder = new StringBuilder(chArray);
    sb.append(' ')
        .append("abcΣΨΩ0123456789")
        .append(' ')
        .append(true)
        .append(' ')
        .append(false)
        .append(' ')
        .append(57 as byte)
        .append(' ')
        .append(127 as byte)
        .append(' ')
        .append(32767 as short)
        .append(' ')
        .append(128934675 as int)
        .append(' ')
        .append(701234987654321 as long)
        .append(' ')
        .append(emptyString)
        .append(nullString);
    return sb.toString();
}

function nestedLoopReserve37() : string {
    let sbOuter = new StringBuilder()      // applied, due loop is countable
    for (let i = 2; i < 8; ++i) {
        sbOuter.append(i)

        let sbInner = new StringBuilder()  // applied, due loop is countable
        for (let j = 2; j < 11; ++j) {
            sbInner.append(j)
        }

        sbOuter.append(sbInner.toString())
    }
    return sbOuter.toString()
}

function storeReserve39() : string {
    sbGlobal = new StringBuilder()            // not applied, due to object being stored
    sbGlobal.append(0)
    sbGlobal.append(1)
    sbGlobal.append(2)
    return sbGlobal.toString()
}

function chainReserve40() : String {
    let chars:FixedArray<char> = new FixedArray<char>(1)
    chars[0] = c'0'
    return new StringBuilder(chars).append(1).append(2).toString()   // applied
}

function chainReserve41(str: string) : String {
    return new StringBuilder(str).append(1).append(2).toString()   // applied
}

function __noinline__foo(sb: StringBuilder) {
    sb.append(1)
}

function argReserve42(): string {
    let sb = new StringBuilder()                // not applied, due to object being passed to non-inlined function
    sb.append(0)
    __noinline__foo(sb)
    sb.append('2')
    return sb.toString();
}

class StringBuilderHolder
{
    sb : StringBuilder = new StringBuilder()
}
let sbHolder = new StringBuilderHolder
function storeReserve43() : string {
    let sb = new StringBuilder()                // not applied, due to object being stored
    sb.append(0)

    sbHolder.sb = sb

    sb.append(1)
    return sb.toString();
}

let sbArray = new StringBuilder[1]
function storeReserve44() : string {
    let sb = new StringBuilder()                // not applied, due to object being stored
    sb.append(0)

    sbArray[0] = sb

    sb.append(1)
    return sb.toString();
}

function mainEventsCheck() : int {
    arktest.assertEQ(tryFinallyReserve31(), '01',
                     'Wrong result at tryFinallyReserve31');
    arktest.assertEQ(outReserve32().append(1).toString(), '01',
                     'Wrong result at outReserve32');
    arktest.assertEQ(inReserve33(new StringBuilder().append(0)), '01',
                     'Wrong result at inReserve33');
    arktest.assertEQ(globalReserve34(), '0',
                     'Wrong result at in_reserve34');
    arktest.assertEQ(chainReserve35(), '01',
                     'Wrong result at chainReserve35');
    arktest.assertEQ(moreTypesReserve36(),
                     '!@#$% abcΣΨΩ0123456789 true false 57 127 32767 128934675 701234987654321 null',
                     'Wrong result at moreTypesReserve36');
    arktest.assertEQ(nestedLoopReserve37(),
                     '223456789103234567891042345678910523456789106234567891072345678910',
                     'Wrong result at nestedLoopReserve37');
    arktest.assertEQ(storeReserve39(), '012',
                     'Wrong result at storeReserve39');
    arktest.assertEQ(chainReserve40(), '012',
                     'Wrong result at chainReserve40');
    arktest.assertEQ(chainReserve41('0'), '012',
                     'Wrong result at chainReserve41');
    arktest.assertEQ(storeReserve43(), '01',
                     'Wrong result at storeReserve43');
    arktest.assertEQ(storeReserve44(), '01',
                     'Wrong result at storeReserve44');

    return 0
}

function mainCompileCheck() : int {
    arktest.assertTrue(tryCatchReserve30(false).startsWith('0'),
                       'Wrong result at tryCatchReserve30');
    arktest.assertTrue(tryCatchReserve30(true).startsWith('01Error'),
                       'Wrong result at tryCatchReserve30');
    arktest.assertEQ(argReserve42(), '012',
                     'Wrong result at argReserve42');
    return 0
}

function main() : int {
    mainEventsCheck()
    mainCompileCheck()
    return 0
}
