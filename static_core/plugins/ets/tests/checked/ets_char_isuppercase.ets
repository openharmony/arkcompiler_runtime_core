/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

//! CHECKER AOT IR Builder inlining Char.isUpperCase()
//! SKIP_IF    @architecture == "arm32"
//! RUN_AOT    options: "--compiler-regex='.*main.*'"
//! METHOD     "ets_char_isuppercase.ETSGLOBAL::callIsUpperCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! METHOD     "ets_char_isuppercase.ETSGLOBAL::callIsLowerCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! METHOD     "ets_char_isuppercase.ETSGLOBAL::callToUpperCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! INST_NEXT  /Mul/
//! INST_NEXT  /Add/
//! METHOD     "ets_char_isuppercase.ETSGLOBAL::callToLowerCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! INST_NEXT  /Mul/
//! INST_NEXT  /Add/

//! CHECKER    Char.isUpperCase() in JIT
//! RUN        force_jit: true, options: "", entry: "ets_char_isuppercase.ETSGLOBAL::main"
//! METHOD     "ets_char_isuppercase.ETSGLOBAL::callIsUpperCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! METHOD     "ets_char_isuppercase.ETSGLOBAL::callIsLowerCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! METHOD     "ets_char_isuppercase.ETSGLOBAL::callToUpperCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! INST_NEXT  /Mul/
//! INST_NEXT  /Add/
//! METHOD     "ets_char_isuppercase.ETSGLOBAL::callToLowerCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! INST_NEXT  /Mul/
//! INST_NEXT  /Add/

function callIsUpperCase(ch: Char): boolean {
  return Char.isUpperCase(ch);
}

function callToUpperCase(ch: Char): char {
  return Char.toUpperCase(ch);
}

function callIsLowerCase(ch: Char): boolean {
  return Char.isLowerCase(ch);
}

function callToLowerCase(ch: Char): char {
  return Char.toLowerCase(ch);
}

function main(): int {
    assertTrue(!callIsUpperCase(c'a'));
    assertTrue(!callIsUpperCase(c'z'));
    assertTrue(callIsUpperCase(c'A'));
    assertTrue(callIsUpperCase(c'Z'));
    assertTrue(!callIsUpperCase(c'+'));

    assertTrue(callIsLowerCase(c'a'));
    assertTrue(callIsLowerCase(c'z'));
    assertTrue(!callIsLowerCase(c'A'));
    assertTrue(!callIsLowerCase(c'Z'));
    assertTrue(!callIsLowerCase(c'+'));

    assertEQ(c'A', callToUpperCase(c'a'));
    assertEQ(c'Z', callToUpperCase(c'z'));
    assertEQ(c'A', callToUpperCase(c'A'));
    assertEQ(c'Z', callToUpperCase(c'Z'));
    assertEQ(c'+', callToUpperCase(c'+'));

    assertEQ(c'a', callToLowerCase(c'a'));
    assertEQ(c'z', callToLowerCase(c'z'));
    assertEQ(c'a', callToLowerCase(c'A'));
    assertEQ(c'z', callToLowerCase(c'Z'));
    assertEQ(c'+', callToLowerCase(c'+'));

    return 0;
}

