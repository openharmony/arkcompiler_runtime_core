/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

//! CHECKER      AOT IR Builder, check null instructions
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex='.*testNull.*' --compiler-inlining=false"
//!
//! METHOD       "ets_nullish.ETSGLOBAL::testNull"
//! PASS_AFTER   "IrBuilder"
//! INST         /LoadUniqueObject/
//!
//! METHOD       "ets_nullish.ETSGLOBAL::testNullDce"
//! PASS_AFTER   "Codegen"
//! INST_NOT     /LoadUniqueObject/
//!  
//! METHOD       "ets_nullish.ETSGLOBAL::testNullCse"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   /LoadUniqueObject/, 1
//!

//! CHECKER      Force JIT null
//! RUN          force_jit: true,
//!              options: "--compiler-regex='.*testNull.*'",
//!              entry: "ets_nullish.ETSGLOBAL::testNull"
//! METHOD       "ets_nullish.ETSGLOBAL::testNull"
//! PASS_AFTER   "IrBuilder"
//! INST         /LoadUniqueObject/

//! CHECKER      Force JIT null dce
//! RUN          force_jit: true,
//!              options: "--compiler-regex='.*testNullDce.*'",
//!              entry: "ets_nullish.ETSGLOBAL::testNullDce"
//! METHOD       "ets_nullish.ETSGLOBAL::testNullDce"
//! PASS_AFTER   "Codegen"
//! INST_NOT     /LoadUniqueObject/

//! CHECKER      Force JIT null cse
//! RUN          force_jit: true,
//!              options: "--compiler-regex='.*testNullCse.*'",
//!              entry: "ets_nullish.ETSGLOBAL::testNullCse"
//! METHOD       "ets_nullish.ETSGLOBAL::testNullCse"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   /LoadUniqueObject/, 1

let glob: Object | undefined | null = null;

function testNull(): int {
    glob = null;
    return 0;
}

function testNullDce(): int {
    let v: Object | null = null;
    return 0;
}

function testNullCse(): int {
    glob = null;
    glob = null;
    return 0;
}
