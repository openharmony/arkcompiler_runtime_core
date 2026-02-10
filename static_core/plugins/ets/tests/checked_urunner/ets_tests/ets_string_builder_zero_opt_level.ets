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

// Check that access to string length in SSB optimization 
// in JIT/AOT mode doesn't crash with es2panda --opt-level=0

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND  options: "--opt-level=0"

//! CHECKER       AOT IR Builder, check String concatenation loop
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*[Cc]oncatLoop[0-9]*' --compiler-inlining=false"

//! CHECKER       JIT IR Builder, check String concatenation loop
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true,
//!               options: "--compiler-regex='.*[Cc]oncatLoop[0-9]+.*' --compiler-inlining=false",
//!               entry: "ets_string_builder_zero_opt_level.ETSGLOBAL::main"

function reuseConcatLoop1(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += str.length;
        str += a;
    }
    return str;
}

function reuseConcatLoop2(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += str.getLength();
        str += a;
    }
    return str;
}

function main() {
    const n: int = 2;
    arktest.assertEQ(reuseConcatLoop1("a", n), "0a2a", "Wrong result at reuseConcatLoop1");
    arktest.assertEQ(reuseConcatLoop2("a", n), "0a2a", "Wrong result at reuseConcatLoop2");
}
