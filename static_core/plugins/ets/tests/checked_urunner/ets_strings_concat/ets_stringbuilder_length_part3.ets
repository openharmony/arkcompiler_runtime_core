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

//! CHECKER     Concat loop with length access unoptimized loop AOT
//! SKIP_IF     @architecture == "arm32"
//! RUN_PAOC    options: "--compiler-regex='.*reuse_concat_loop1' --compiler-optimize-string-concat=true --compiler-inlining=false --compiler-check-final=false --compiler-simplify-string-builder=false"
//! RUN         options: "--compiler-regex='.*reuse_concat_loop1'",  entry: "ets_stringbuilder_length_part3.ETSGLOBAL::main", result: 0

//! CHECKER     Concat loop with length access loop AOT CHA inline
//! SKIP_IF     @architecture == "arm32"
//! RUN_PAOC    options: "--compiler-regex='.*reuse_concat_loop1' --compiler-optimize-string-concat=true --compiler-inlining=true --paoc-use-cha=true --compiler-check-final=false"
//!
//! METHOD      "ets_stringbuilder_length_part3.ETSGLOBAL::reuse_concat_loop1"
//! PASS_BEFORE "SimplifyStringBuilder"
//! INST_COUNT  /StringBuilder::<ctor>/,1
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,1
//! INST_COUNT  /LenArray/,0
//! INST_COUNT  /LoadObject \d+ std.core.StringBuilder.length/,0
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /StringBuilder::<ctor>/,1
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,1
//! INST_NOT    /LenArray/
//! INST_COUNT  /LoadObject \d+ std.core.StringBuilder.length/,0
//!
//! RUN         options: "--compiler-regex='.*reuse_concat_loop1'", entry: "ets_stringbuilder_length_part3.ETSGLOBAL::main", result: 0

function reuse_concat_loop1(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += str.length;                      // partially applied
        str += a;                               // two SBs merged into one
    }
    return str;
}

function main() {
    arktest.assertEQ(reuse_concat_loop1("a", 4), "0a2a4a6a", "Wrong result at reuse_concat_loop1");
}
