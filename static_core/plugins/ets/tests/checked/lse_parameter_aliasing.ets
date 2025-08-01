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

//! CHECKER      Check that LSE and AA work correctly with aliased parameters
//! RUN          force_jit: true, options: "--compiler-regex='.*test.*'", entry: "lse_parameter_aliasing.ETSGLOBAL::main"
//! METHOD       "lse_parameter_aliasing.ETSGLOBAL::test"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "LoadArray", 3
//! PASS_BEFORE  "Lowering"
//! INST_COUNT   "LoadArray", 3

//! CHECKER      Check that LSE and AA work correctly with aliased parameters (AOT)
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex='.*test.*'"
//! METHOD       "lse_parameter_aliasing.ETSGLOBAL::test"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "LoadArray", 3
//! PASS_BEFORE  "Lowering"
//! INST_COUNT   "LoadArray", 3
//! RUN          options: "", entry: "lse_parameter_aliasing.ETSGLOBAL::main"

function test(a: FixedArray<FixedArray<String>>, b: FixedArray<FixedArray<String>>): String {
    let x = a[0]
    b[0] = new String[2]
    let y = a[0]
    x[0] = "a"
    return y[0]
  }

function main(): void
{
    {
        let c: FixedArray<FixedArray<String>> = new String[2][2]
        let d: FixedArray<FixedArray<String>> = new String[2][2]
        c[0][0] = "b"
        assertEQ(test(c, d), "a")
        assertEQ(c[0][0], "a")
    }
    {
        let c: FixedArray<FixedArray<String>> = new String[2][2]
        c[0][0] = "b"
        assertEQ(test(c, c), "")
        assertEQ(c[0][0], "")
    }
}
