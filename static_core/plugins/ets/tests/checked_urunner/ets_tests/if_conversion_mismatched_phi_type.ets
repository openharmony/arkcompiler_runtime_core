/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

//! CHECKER       IfConversion handles XorI.b going into Phi.i32 AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='if_conversion_mismatched_phi_type.ETSGLOBAL::foo'"
//! METHOD        "if_conversion_mismatched_phi_type.ETSGLOBAL::foo"
//! PASS_BEFORE   "IfConversion"
//! INST_COUNT    /b\s+XorI/, 3
//! INST          /i32\s+Phi/
//! INST          /b\s+Phi/
//! PASS_AFTER    "IfConversion"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT      /b\s+Phi/
//! INST_NOT      /i32\s+Phi/
//! INST          /b\s+XorI/
//! INST_NEXT     /i32\s+SelectImm EQ b/
//! INST_NEXT     /i32\s+XorI/
//! INST_NEXT     /b\s+XorI/
//! INST_NEXT     /b\s+SelectImm EQ i32/
//! RUN           entry: "if_conversion_mismatched_phi_type.ETSGLOBAL::main"

function foo(flagA: boolean, flagB: boolean, flagC: boolean) {
    return !flagA && !flagB && !flagC
}

function main(): void {
    arktest.assertEQ(foo(false, false, false), true);
    arktest.assertEQ(foo(false, false, true), false);
    arktest.assertEQ(foo(false, true, false), false);
    arktest.assertEQ(foo(false, true, true), false);
    arktest.assertEQ(foo(true, false, false), false);
    arktest.assertEQ(foo(true, false, true), false);
    arktest.assertEQ(foo(true, true, false), false);
    arktest.assertEQ(foo(true, true, true), false);
}
