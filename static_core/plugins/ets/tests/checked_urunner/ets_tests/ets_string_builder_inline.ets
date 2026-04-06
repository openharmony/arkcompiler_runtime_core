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

//! CHECKER       AOT. Check that SimplifyStringBuilder inline methods
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-inlining=true --compiler-regex=.*main.*"
//!
//! METHOD        "ets_string_builder_inline.ETSGLOBAL::main"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /std.core.StringBuilder::%%get-stringLength/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",0
//! INST_COUNT    /StoreObject \d+ std.core.StringBuilder.buf/,1
//! INST_COUNT    /StoreObject \d+ std.core.StringBuilder.index/,1
//! INST_COUNT    /StoreObject \d+ std.core.StringBuilder.length/,1
//! INST_COUNT    /StoreObject \d+ std.core.StringBuilder.compress/,1
//! INST_COUNT    /std.core.StringBuilder::%%get-stringLength/,0
//! INST_COUNT    /LoadObject \d+ std.core.StringBuilder.length/,1

//! CHECKER       AOT. Check that SimplifyStringBuilder doesn't inline methods if inlining is disabled
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-inlining=false --compiler-regex=.*main.*"
//!
//! METHOD        "ets_string_builder_inline.ETSGLOBAL::main"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /std.core.StringBuilder::%%get-stringLength/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /std.core.StringBuilder::%%get-stringLength/,1

//! CHECKER       AOT. Check that SimplifyStringBuilder doesn't inline methods if methods in blacklist
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-inlining-blacklist='std.core.StringBuilder::<ctor>," +
//!                        "std.core.StringBuilder::%%get-stringLength'" +
//!                        " --compiler-regex=.*main.*"
//!
//! METHOD        "ets_string_builder_inline.ETSGLOBAL::main"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /std.core.StringBuilder::%%get-stringLength/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /std.core.StringBuilder::%%get-stringLength/,1

//! CHECKER       AOT. Check that methods are inlined if SimplifyStringBuilder is disabled
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-simplify-string-builder=false --compiler-regex=.*main.*"
//!
//! METHOD        "ets_string_builder_inline.ETSGLOBAL::main"
//! PASS_BEFORE   "Inline"
//! INST_COUNT    /CallStatic \d+ std.core.StringBuilder::<ctor>/,1
//! INST_COUNT    /CallStatic \d+ std.core.StringBuilder::%%get-stringLength/,1
//! PASS_AFTER    "Inline"
//! INST_COUNT    /CallStatic.Inlined \d+ std.core.StringBuilder::<ctor>/,1
//! INST_COUNT    /CallStatic.Inlined \d+ std.core.StringBuilder::%%get-stringLength/,1

//! CHECKER       JIT. Check that SimplifyStringBuilder inline methods
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true,
//!               options: "--compiler-inlining=true --compiler-regex=.*main.*",
//!               entry: "ets_string_builder_inline.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_builder_inline.ETSGLOBAL::main"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /std.core.StringBuilder::%%get-stringLength/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",0
//! INST_COUNT    /StoreObject \d+ std.core.StringBuilder.buf/,1
//! INST_COUNT    /StoreObject \d+ std.core.StringBuilder.index/,1
//! INST_COUNT    /StoreObject \d+ std.core.StringBuilder.length/,1
//! INST_COUNT    /StoreObject \d+ std.core.StringBuilder.compress/,1
//! INST_COUNT    /std.core.StringBuilder::%%get-stringLength/,0
//! INST_COUNT    /LoadObject \d+ std.core.StringBuilder.length/,1

//! CHECKER       JIT. Check that SimplifyStringBuilder doesn't inline methods if inlining is disabled
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true,
//!               options: "--compiler-inlining=false --compiler-regex=.*main.*",
//!               entry: "ets_string_builder_inline.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_builder_inline.ETSGLOBAL::main"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /std.core.StringBuilder::%%get-stringLength/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /std.core.StringBuilder::%%get-stringLength/,1

//! CHECKER       JIT. Check that SimplifyStringBuilder doesn't inline methods if methods in blacklist
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true,
//!               options: "--compiler-inlining-blacklist='std.core.StringBuilder::<ctor>," +
//!                        "std.core.StringBuilder::%%get-stringLength'",
//!               entry: "ets_string_builder_inline.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_builder_inline.ETSGLOBAL::main"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /std.core.StringBuilder::%%get-stringLength/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /std.core.StringBuilder::%%get-stringLength/,1

//! CHECKER       JIT. Check that methods are inlined if SimplifyStringBuilder is disabled
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true,
//!               options: "--compiler-simplify-string-builder=false --compiler-regex=.*main.*",
//!               entry: "ets_string_builder_inline.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_builder_inline.ETSGLOBAL::main"
//! PASS_BEFORE   "Inline"
//! INST_COUNT    /CallStatic \d+ std.core.StringBuilder::<ctor>/,1
//! INST_COUNT    /CallStatic \d+ std.core.StringBuilder::%%get-stringLength/,1
//! PASS_AFTER    "Inline"
//! INST_COUNT    /CallStatic.Inlined \d+ std.core.StringBuilder::<ctor>/,1
//! INST_COUNT    /CallStatic.Inlined \d+ std.core.StringBuilder::%%get-stringLength/,1

function main(){
    let sb = new StringBuilder();
    let sbLen = sb.stringLength;
    return sbLen;
}
