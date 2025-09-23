/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

//! CHECKER       AOT IR Builder, check String creation from a single and numerous char code(s)
//! RUN_PAOC      options: "--compiler-regex='.*from.*CharCode.*' --compiler-inlining=false"
//!
//! METHOD        "ets_string_from_char_code.ETSGLOBAL::fromCharCodes"
//! PASS_AFTER    "Peephole"
//! INST          /Intrinsic.StdCoreStringFromCharCode/
//! INST_NOT      /Intrinsic.StdCoreStringFromCharCodeSingle/
//!
//! METHOD        "ets_string_from_char_code.ETSGLOBAL::fromLargeCharCode"
//! PASS_AFTER    "Peephole"
//! INST          /Intrinsic.StdCoreStringFromCharCodeSingle/

//! CHECKER       AOT IR Builder, check String creation from a single and numerous char code(s) with no string caches
//! RUN_PAOC      options: "--compiler-regex='.*from.*CharCode.*' --compiler-inlining=false --use-string-caches=false"
//!
//! METHOD        "ets_string_from_char_code.ETSGLOBAL::fromCharCodes"
//! PASS_AFTER    "Peephole"
//! INST          /Intrinsic.StdCoreStringFromCharCode/
//! INST_NOT      /Intrinsic.StdCoreStringFromCharCodeSingle/
//!
//! METHOD        "ets_string_from_char_code.ETSGLOBAL::fromLargeCharCode"
//! PASS_AFTER    "Peephole"
//! INST          /Intrinsic.StdCoreStringFromCharCodeSingle/

//! CHECKER       JIT IR Builder, check String creation from a single and numerous char code(s)
//! RUN           force_jit: true, options: "--compiler-regex='.*from.*CharCode.*' --compiler-inlining=false", entry: "ets_string_from_char_code.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_from_char_code.ETSGLOBAL::fromCharCodes"
//! PASS_AFTER    "Peephole"
//! INST          /Intrinsic.StdCoreStringFromCharCode/
//! INST_NOT      /Intrinsic.StdCoreStringFromCharCodeSingle/
//!
//! METHOD        "ets_string_from_char_code.ETSGLOBAL::fromLargeCharCode"
//! PASS_AFTER    "Peephole"
//! INST          /Intrinsic.StdCoreStringFromCharCodeSingle/

function fromCharCodes(code1: number, code2: number): string {
    return string.fromCharCode(code1, code2);
}

function fromLargeCharCode(code: int): string {
    return string.fromCharCode(Int.toDouble(code << 8 | code));
}

function fromOneByteCharCode(code: int): string {
    arktest.assertLT(code, 128);
    return string.fromCharCode(Int.toDouble(code));
}

function main() {
    arktest.assertEQ(fromCharCodes(0xd842, 0xdfb7), '𠮷');
    console.log((0x42 << 8 | 0x42));
    console.log(string.fromCharCode(Int.toDouble(0x42 << 8 | 0x42)));
    arktest.assertEQ(fromLargeCharCode(0x42), '䉂'); // 0x4242
    arktest.assertEQ(fromOneByteCharCode(0x41), 'A');
}
