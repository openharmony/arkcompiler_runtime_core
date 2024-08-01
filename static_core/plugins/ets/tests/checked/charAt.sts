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

//! CHECKER      Test charAt inlining in compiler
//! RUN          force_jit: true, options: "", entry: "ETSGLOBAL::main"
//! METHOD       "ETSGLOBAL::main"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "Intrinsic.StdCoreStringCharAt"

function main(): void {
    let str: String = new String("aBCd");
    assert str.charAt(0) == c'a': "Wrong str.charAt(0)";
    assert str.charAt(1) == c'B': "Wrong str.charAt(1)";
    assert str.charAt(2) == c'C': "Wrong str.charAt(2)";
    assert str.charAt(3) == c'd': "Wrong str.charAt(3)";
}
