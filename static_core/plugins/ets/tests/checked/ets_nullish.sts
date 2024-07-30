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

//! CHECKER      AOT IR Builder, check undefined instructions
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex='.*test_undefined.*' --compiler-inlining=false"
//!
//! METHOD       "ETSGLOBAL::test_undefined"
//! PASS_AFTER   "IrBuilder"
//! INST         /LoadUndefined/
//!
//! METHOD       "ETSGLOBAL::test_undefined_dce"
//! PASS_AFTER   "Codegen"
//! INST_NOT     /LoadUndefined/
//!  
//! METHOD       "ETSGLOBAL::test_undefined_cse"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   /LoadUndefined/, 1
//!

//! CHECKER      Force JIT undefined
//! RUN          force_jit: true, options: "--compiler-regex='.*test_undefined.*'", entry: "ETSGLOBAL::test_undefined"
//! METHOD       "ETSGLOBAL::test_undefined"
//! PASS_AFTER   "IrBuilder"
//! INST         /LoadUndefined/

//! CHECKER      Force JIT undefined dce
//! RUN          force_jit: true, options: "--compiler-regex='.*test_undefined_dce.*'", entry: "ETSGLOBAL::test_undefined_dce"
//! METHOD       "ETSGLOBAL::test_undefined_dce"
//! PASS_AFTER   "Codegen"
//! INST_NOT     /LoadUndefined/

//! CHECKER      Force JIT undefined cse
//! RUN          force_jit: true, options: "--compiler-regex='.*test_undefined_cse.*'", entry: "ETSGLOBAL::test_undefined_cse"
//! METHOD       "ETSGLOBAL::test_undefined_cse"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   /LoadUndefined/, 1

let glob: Object | null | undefined = null;

function test_undefined(): int {
    glob = undefined;
    return 0;
}

function test_undefined_dce(): int {
    let v: Object | undefined = undefined;
    return 0;
}

function test_undefined_cse(): int {
    glob = undefined;
    glob = undefined;
    return 0;
}
