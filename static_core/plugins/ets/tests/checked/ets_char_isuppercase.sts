/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
//! METHOD     "ETSGLOBAL::callIsUpperCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! METHOD     "ETSGLOBAL::callIsLowerCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! METHOD     "ETSGLOBAL::callToUpperCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! INST_NEXT  /Mul/
//! INST_NEXT  /Add/
//! METHOD     "ETSGLOBAL::callToLowerCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! INST_NEXT  /Mul/
//! INST_NEXT  /Add/

//! CHECKER    Char.isUpperCase() in JIT
//! RUN        force_jit: true, options: "", entry: "ETSGLOBAL::main"
//! METHOD     "ETSGLOBAL::callIsUpperCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! METHOD     "ETSGLOBAL::callIsLowerCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! METHOD     "ETSGLOBAL::callToUpperCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! INST_NEXT  /Mul/
//! INST_NEXT  /Add/
//! METHOD     "ETSGLOBAL::callToLowerCase"
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
    assert !callIsUpperCase(c'a');
    assert !callIsUpperCase(c'z');
    assert  callIsUpperCase(c'A');
    assert  callIsUpperCase(c'Z');
    assert !callIsUpperCase(c'+');

    assert  callIsLowerCase(c'a');
    assert  callIsLowerCase(c'z');
    assert !callIsLowerCase(c'A');
    assert !callIsLowerCase(c'Z');
    assert !callIsLowerCase(c'+');

    assert c'A' == callToUpperCase(c'a');
    assert c'Z' == callToUpperCase(c'z');
    assert c'A' == callToUpperCase(c'A');
    assert c'Z' == callToUpperCase(c'Z');
    assert c'+' == callToUpperCase(c'+');

    assert c'a' == callToLowerCase(c'a');
    assert c'z' == callToLowerCase(c'z');
    assert c'a' == callToLowerCase(c'A');
    assert c'z' == callToLowerCase(c'Z');
    assert c'+' == callToLowerCase(c'+');

    return 0;
}

