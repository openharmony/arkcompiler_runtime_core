/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

function EscapeTest() {
    const escapeSeqGolden = "A%20B%u1234%00%20C";
    const escapeSeqActual = escape(String.fromCharCode(0x41, 0x20, 0x42, 0x1234, 0, 0x20, 0x43));
    assertEQ(escapeSeqActual, escapeSeqGolden, "escapeSeqActual must be equal to escapeSeqGolden");
}

function OverflowSingleCasesTest() {
    // on arm32 this case should be disabled due to
    // https://gitee.com/openharmony/arkcompiler_runtime_core/issues/IBSVYX
    assertEQ(string.fromCharCode(NaN), "\u{0}", "CharCode[0] of a string built from NaN must be 0");
    assertEQ(string.fromCharCode(NaN).length, 1);
    assertEQ(string.fromCharCode(Infinity), "\u{0}", "CharCode[0] of a string built from Infinity must be 0");
    assertEQ(string.fromCharCode(Infinity).length, 1);
}

function main(): int {
    let fromCharCodeEscapeTestsuite = new ArkTestsuite("intrinsics.string_from_char_code_escape");
    fromCharCodeEscapeTestsuite.addTest("EscapeTest", EscapeTest);
    fromCharCodeEscapeTestsuite.addTest("OverflowSingleCases", OverflowSingleCasesTest);
    return fromCharCodeEscapeTestsuite.run();
}