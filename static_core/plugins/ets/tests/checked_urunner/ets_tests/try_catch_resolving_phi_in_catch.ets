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

//! CHECKER      JIT TryCatchResolving doesn't crash with a phi in a catch block
//! RUN          options: "--compiler-enable-jit=true --no-async-jit=true --compiler-hotness-threshold=20 \
//!                        --compiler-profiling-threshold=0 --compiler-enable-osr=false \
//!                        --compiler-regex=try_catch_resolving_phi_in_catch.ETSGLOBAL::phiInCatch",
//!              entry: "try_catch_resolving_phi_in_catch.ETSGLOBAL::main"
//! METHOD       "try_catch_resolving_phi_in_catch.ETSGLOBAL::phiInCatch"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Throw", 2
//! IN_BLOCK     /\bcatch,/, 2
//! INST         /i32\s+Phi/
//! PASS_AFTER   "TryCatchResolving"
//! INST_NOT     "Throw"
//! INST_NOT     /i32\s+Phi/

//! CHECKER      AOT-PGO TryCatchResolving doesn't crash with a phi in a catch block
//! RUN_PGO_PROF entry: "try_catch_resolving_phi_in_catch.ETSGLOBAL::main"
//! RUN_PGO_PAOC options: "--compiler-regex=try_catch_resolving_phi_in_catch.ETSGLOBAL::phiInCatch"
//! METHOD       "try_catch_resolving_phi_in_catch.ETSGLOBAL::phiInCatch"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Throw", 2
//! IN_BLOCK     /\bcatch,/, 2
//! INST         /i32\s+Phi/
//! PASS_AFTER   "TryCatchResolving"
//! INST_NOT     "Throw"
//! INST_NOT     /i32\s+Phi/
//! RUN          entry: "try_catch_resolving_phi_in_catch.ETSGLOBAL::main"

class Exc extends Error {}

function phiInCatch(): int {
    let a: int = 0;
    try {
        try {
            throw new Error();
        } catch (e) {
            arktest.assertEQ(e instanceof Exc, false);
        } finally {
            a = 1;
        }
    } catch (e) {}
    return a;
}

function main(): int {
    for (let i: int = 0; i < 30; i++) {
        arktest.assertEQ(phiInCatch(), 1);
    }
    return 0;
}
