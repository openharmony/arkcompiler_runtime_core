/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND  options: "--ets-strings-concat=false --bco-compiler --compiler-simplify-string-builder=false"

//! CHECKER       AOT IR Builder, check String concatenation loop
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*[Cc]oncatLoop[0-9]*' --compiler-inlining=false"
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop0"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop1"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop2"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /head, loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /head, loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop3"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop4"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop5"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop7"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::equivalentConcatLoop7"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop8"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "Codegen"
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::nestedConcatLoop0"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 3/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 3/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 2/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::tryCatchConcatLoop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,8
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,3
//! INST_COUNT    /Intrinsic.StdCoreStringConcat4/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::tryCatchConcatLoop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::tryCatchConcatLoop2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,2
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::tryCatchConcatLoop3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::tryCatchConcatLoop4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::tryCatchConcatLoop5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,5
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,5
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,5
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,2
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,4
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop7"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop9"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop10"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop11"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::switchConcatLoop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::switchConcatLoop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuseConcatLoop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,0
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuseConcatLoop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuseConcatLoop2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuseConcatLoop3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuseConcatLoop4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuseConcatLoop5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::argsConcatLoop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendBool/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendChar/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendBool/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendChar/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! RUN           options: "--compiler-regex='.*[Cc]oncatLoop[0-9]*'",
//!               entry: "ets_string_concat_loop.ETSGLOBAL::main"

//! CHECKER       JIT IR Builder, check String concatenation loop
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true,
//!               options: "--compiler-regex='.*[Cc]oncatLoop[0-9]+.*' --compiler-inlining=false",
//!               entry: "ets_string_concat_loop.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop0"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop1"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop2"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /head, loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /head, loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop3"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop4"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop5"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop7"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::equivalentConcatLoop7"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop8"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "Codegen"
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::nestedConcatLoop0"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 3/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 3/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 2/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::tryCatchConcatLoop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,8
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,3
//! INST_COUNT    /Intrinsic.StdCoreStringConcat4/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::tryCatchConcatLoop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::tryCatchConcatLoop2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,2
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::tryCatchConcatLoop3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::tryCatchConcatLoop4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::tryCatchConcatLoop5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,5
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,5
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,5
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,2
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,4
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop7"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop9"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop10"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop11"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::switchConcatLoop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::switchConcatLoop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuseConcatLoop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,0
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuseConcatLoop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuseConcatLoop2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuseConcatLoop3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuseConcatLoop4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuseConcatLoop5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::argsConcatLoop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendBool/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendChar/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendBool/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendChar/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2

//! CHECKER       Invalid save states JIT (#I932LA/15833)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true,
//!               options: "--compiler-inlining=false --compiler-regex='.*bugfixConcatLoop1'",
//!               entry: "ets_string_concat_loop.ETSGLOBAL::main"
//!
//! METHOD        "bugfixConcatLoop1"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 1/
//! INST          /StringBuilder::<ctor>/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1/
//! INST_NOT      /StringBuilder::<ctor>/

//! CHECKER       Invalid save states AOT (#I932LA/15833)
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*bugfixConcatLoop1'"
//!
//! METHOD        "bugfixConcatLoop1"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 1/
//! INST          /StringBuilder::<ctor>/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1/
//! INST_NOT      /StringBuilder::<ctor>/
//!
//! RUN           options: "--compiler-regex='.*bugfixConcatLoop1'", entry: "ets_string_concat_loop.ETSGLOBAL::main"


//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN_BCO       options: "--ets-strings-concat=false --bco-compiler --compiler-simplify-string-builder=true"

//! CHECKER       BCO, check String concatenation loop
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop0"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::append/,1
//! INST          /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::append/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::append/
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::toString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop1"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::append/,1
//! INST          /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::append/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::append/
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::toString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop2"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /head, loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::append/,1
//! INST          /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::append/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /head, loop/
//! INST          /StringBuilder::append/
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::toString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop3"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::append/,1
//! INST          /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::append/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::append/
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::toString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop4"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::append/,1
//! INST          /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::append/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::append/
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::toString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop5"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::append/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::append/,2
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::toString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop7"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,0
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::append/,2
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::equivalentConcatLoop7"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,4
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::append/,2
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concatLoop8"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /StringBuilder::append/,6
//! INST_COUNT    /StringBuilder::toString/,6
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,0
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::append/,6
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::toString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::nestedConcatLoop0"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop 3/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::append/,1
//! INST          /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::append/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 3/
//! INST          /StringBuilder::append/
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 2/
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1/
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::toString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::tryCatchConcatLoop0"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /StringBuilder::append/,8
//! INST_COUNT    /StringBuilder::toString/,6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /StringBuilder::append/,8
//! INST_COUNT    /StringBuilder::toString/,6
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::tryCatchConcatLoop1"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::tryCatchConcatLoop2"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::tryCatchConcatLoop3"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::tryCatchConcatLoop4"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,1
//! INST_COUNT    /StringBuilder::toString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,1
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::tryCatchConcatLoop5"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,5
//! INST_COUNT    /StringBuilder::append/,5
//! INST_COUNT    /StringBuilder::toString/,5
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,4
//! INST_COUNT    /StringBuilder::append/,5
//! INST_COUNT    /StringBuilder::toString/,5
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop0"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,4
//! INST_COUNT    /StringBuilder::append/,4
//! INST_COUNT    /StringBuilder::toString/,4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,4
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop1"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop2"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop3"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop4"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop5"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop6"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop7"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop8"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop9"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop10"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop 1, depth 1/,2
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,1
//! INST_COUNT    /StringBuilder::toString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,0
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1, depth 1/,2
//! INST_COUNT    /StringBuilder::append/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::ifConcatLoop11"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop 1, depth 1/,2
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,1
//! INST_COUNT    /StringBuilder::toString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,0
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1, depth 1/,2
//! INST_COUNT    /StringBuilder::append/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::switchConcatLoop0"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /StringBuilder::append/,6
//! INST_COUNT    /StringBuilder::toString/,6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,6
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::switchConcatLoop1"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /StringBuilder::append/,6
//! INST_COUNT    /StringBuilder::toString/,6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,6
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuseConcatLoop0"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuseConcatLoop1"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuseConcatLoop2"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuseConcatLoop3"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuseConcatLoop4"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuseConcatLoop5"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::argsConcatLoop0"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,4
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,4
//! INST_COUNT    /StringBuilder::toString/,2

//! CHECKER       INTERP-IRTOC, check String concatenation loop
//! SKIP_IF       @architecture == "arm32"
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN           options: "--compiler-regex='.*[Cc]oncatLoop[0-9]*' --compiler-enable-jit=false \
//!                         --interpreter-type=irtoc --compiler-enable-events --compiler-events-path=./events.csv",
//!               entry: "ets_string_concat_loop.ETSGLOBAL::main"

//! CHECKER       INTERP-CPP, check String concatenation loop
//! SKIP_IF       @architecture == "arm32"
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN           options: "--compiler-regex='.*[Cc]oncatLoop[0-9]*' --compiler-enable-jit=false \
//!                         --interpreter-type=cpp --compiler-enable-events --compiler-events-path=./events.csv",
//!               entry: "ets_string_concat_loop.ETSGLOBAL::main"

function concatLoop0(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i)
        str = str + a;                              // applied
    return str;
}

function concatLoop1(a: String, n: int): String {
    let str: String = "";
    while (--n >= 0)
        str += a;                                   // applied
    return str;
}

function concatLoop2(a: String, n: int): String {
    let str: String = "";
    do
        str += a;                                   // applied
    while (--n > 0);
    return str;
}

function concatLoop3(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i, str += a);          // applied
    return str;
}

function concatLoop4(strings: FixedArray<string>): String {
    let str: String = "";
    for (let i = 0; i < strings.length; ++i) {
        str += strings[i];                          // applied
    }
    return str;
}

function concatLoop5(strings: FixedArray<string>): String {
    let str: String = "";
    for (let i = 0; i < strings.length; ++i) {
        str += strings[0] + strings[i];             // applied
    }
    return str;
}

function concatLoop7(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                                   // applied
        str += b;                                   // applied
    }
    return str;
}

function equivalentConcatLoop7(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        let sb0 = new StringBuilder();              // applied
        sb0.append(str);
        sb0.append(a);
        let tmp1 = sb0.toString();

        let sb1 = new StringBuilder();              // applied
        sb1.append(tmp1);
        sb1.append(b);
        str = sb1.toString();
    }
    return str;
}

function concatLoop8(a: String, b: String, n: int): String {
    let a1b: String = "";
    let b2a: String = "";
    for (let i = 0; i < n; ++i) {
        a1b += a + 1;                               // applied
        a1b += b;                                   // applied

        b2a += b + 2;                               // applied
        b2a += a;                                   // applied
    }
    return a1b + "." + b2a;
}

function nestedConcatLoop0(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i)
        for (let j = 0; j < n; ++j)
            for (let k = 0; k < n; ++k)
                str = str + a;                      // applied, instructions moved out of outermost loop
    return str;
}

function tryCatchConcatLoop0(a: String, n: int): String {
    let strTry: String = "";
    let strCatch: String = "";
    let strFinally: String = "";
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                                   // not applied due to loop containing try-catch-finally blocks
                                                    // replaced by concat intrinsic
        try {
            strTry += a;                           // not applied due to being inside try-block
                                                    // replaced by concat intrinsic
        }
        catch(ex) {
            strCatch += a;                         // not applied due to being inside catch-block
                                                    // replaced by concat intrinsic
        }
        finally {
            strFinally += a;                       // not applied due to being inside finally-block
                                                    // replaced by concat intrinsic
        }
    }
    return str + strTry + strCatch + strFinally; // replaced by concat intrinsic
}

function tryCatchConcatLoop1(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i)
        try {
            str = str + a;                          // not applied
                                                    // replaced by concat intrinsic
        }
        catch(ex) {
            str = str + a;                          // not applied
                                                    // replaced by concat intrinsic
        }
    return str;
}

function tryCatchConcatLoop2(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i)
        try {
            str = str + a;                          // not applied
                                                    // replaced by concat intrinsic
        }
        finally {
            str = str + a;                          // partially applied
                                                    // SB merged with the one inside try-block
        }
    return str;
}

function tryCatchConcatLoop3(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i)
        try {
            for (let j = 0; j < n; ++j) {
                str += a;                       // not applied due to loop containing try-catch-finally blocks
                                                // replaced by concat intrinsic
            }
        }
        catch(ex) {
            for (let j = 0; j < n; ++j) {
                str += a;                       // not applied
                                                // replaced by concat intrinsic
            }
        }
    return str;
}

function tryCatchConcatLoop4(a: String, n: int): String {
    let str: String = "";
    try {
        for (let i = 0; i < n; ++i)
            str += a;                           // not applied due to loop containing try-catch-finally blocks
                                                // replaced by concat intrinsic
    }
    catch(ex) {}
    return str;
}

function tryCatchConcatLoop5(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                   // not applied due to loop containing try-catch-finally blocks
                                    // replaced by concat intrinsic
        try {
            str += a;               // partially applied
                                    // SBs inside try, catch, and finally blocks merged into one
        }
        catch(ex) {
            str += a;               // partially applied
                                    // SBs inside try, catch, and finally blocks merged into one
        }
        finally {
            str += a;               // partially applied
                                    // SBs inside try, catch, and finally blocks merged into one
        }
    }
    return str;
}

function ifConcatLoop0(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        if (i < n/2)
            str += 1;                           // applied
        else
            str += 2;                           // applied
        str += b;                               // applied
    }
    return str;
}

function ifConcatLoop1(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        if (i < n/2)
            str += 1;                           // applied
        else
            str += 2;                           // applied
    }
    return str;
}

function ifConcatLoop2(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        if (i < n/2)
            str += 1;                           // applied
        str += b;                               // applied
    }
    return str;
}

function ifConcatLoop3(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        if (i < n/2) {
        }
        else
            str += 2;                           // applied
        str += b;                               // applied
    }
    return str;
}

function ifConcatLoop4(b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        if (i < n/2)
            str += 1;                           // applied
        else
            str += 2;                           // applied
        str += b;                               // applied
    }
    return str;
}

function ifConcatLoop5(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        if (i < n/2)
            str += 1;                           // applied
    }
    return str;
}

function ifConcatLoop6(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        if (i < n/2) {
        }
        else
            str += 2;                           // applied
    }
    return str;
}

function ifConcatLoop7(n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        if (i < n/2)
            str += 1;                           // applied
        else
            str += 2;                           // applied
    }
    return str;
}

function ifConcatLoop8(b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        if (i < n/2) {
        }
        else
            str += 2;                           // applied
        str += b;                               // applied
    }
    return str;
}

function ifConcatLoop9(b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        if (i < n/2)
            str += 1;                           // applied
        str += b;                               // applied
    }
    return str;
}

function ifConcatLoop10(n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        if (i < n/2)
            str += 1;                           // applied
    }
    return str;
}

function ifConcatLoop11(n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        if (i < n/2) {
        }
        else
            str += 2;                           // applied
    }
    return str;
}

function switchConcatLoop0(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        switch (i) {
            case 0:
                str += 0;                       // applied
                break;
            case 1:
                str += 1;                       // applied
                break;
            case 2:
                str += 2;                       // applied
                break;
            default:
                str += i;                       // applied
                break;
        }
        str += b;                               // applied
    }
    return str;
}

function switchConcatLoop1(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        switch (i) {
            case 0:
                str += 0;                       // applied
            case 1:
                str += 1;                       // applied
            case 2:
                str += 2;                       // applied
            default:
                str += i;                       // applied
        }
        str += b;                               // applied
    }
    return str;
}

function reuseConcatLoop0(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += str;                             // partially applied
        str += a;                               // two SBs merged into one
    }
    return str;
}

function reuseConcatLoop1(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += str.length;                      // partially applied
        str += a;                               // two SBs merged into one
    }
    return str;
}

function reuseConcatLoop2(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += str.getLength();                 // partially applied
        str += a;                               // two SBs merged into one
    }
    return str;
}

function reuseConcatLoop3(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // not applied
        str += str;                             // unsupported string concatenation pattern
    }
    return str;
}

function reuseConcatLoop4(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // partially applied
        str += str.length;                      // two SBs merged into one
    }
    return str;
}

function reuseConcatLoop5(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // partially applied
        str += str.getLength();                 // two SBs merged into one
    }
    return str;
}

function argsConcatLoop0(x: double, y: boolean, ...p: FixedArray<char>): string
{
    let res: string = "" + x + y + p.length;    // not replaced by intrinsic due to concatenating float, bool, int
    for (let ch of p) res += ch;                // not applied due to exit block not dominated by preheader
    return res;
}

function bugfixConcatLoop0(): String {
    let res = ""
    for (let i = 0; i < 1; ++i)
    {
        if (i < 0) {
            res += "0"
        }
        else
        {
            res += "12345"
            res += "678"
        }
    }
    return res
}

function bugfixConcatLoop1(size: int, charset: string): string {
    let res: string = ''

    switch (charset) {
        case "ISO-8859-1":
            for (let i = 0; i < size; ++i)
                res += string.fromCharCode(0x43)
        break
        case "UTF-16":
            for (let i = 0; i < size; ++i)
                res += string.fromCharCode(0xD799)
        break
    }

    return res
}

function main() {
    const n: int = 2;
    arktest.assertEQ(concatLoop0("a", n), "aa", "Wrong result at concatLoop0");
    arktest.assertEQ(concatLoop1("a", n), "aa", "Wrong result at concatLoop1");
    arktest.assertEQ(concatLoop2("a", n), "aa", "Wrong result at concatLoop2");
    arktest.assertEQ(concatLoop3("a", n), "aa", "Wrong result at concatLoop3");
    arktest.assertEQ(concatLoop4(["a", "b", "c", "d"]), "abcd", "Wrong result at concatLoop4");
    arktest.assertEQ(concatLoop5(["a", "b", "c", "d"]), "aaabacad", "Wrong result at concatLoop5");
    arktest.assertEQ(concatLoop7("a", "b", n), "abab", "Wrong result at concatLoop7");
    arktest.assertEQ(equivalentConcatLoop7("a", "b", n), "abab", "Wrong result at equivalentConcatLoop7");
    arktest.assertEQ(concatLoop8("a", "b", n), "a1ba1b.b2ab2a", "Wrong result at concatLoop8");
    arktest.assertEQ(nestedConcatLoop0("a", n), "aaaaaaaa", "Wrong result at nestedConcatLoop0");
    arktest.assertEQ(tryCatchConcatLoop0("a", n), "aaaaaa", "Wrong result at tryCatchConcatLoop0");
    arktest.assertEQ(tryCatchConcatLoop1("a", n), "aa", "Wrong result at tryCatchConcatLoop1");
    arktest.assertEQ(tryCatchConcatLoop2("a", n), "aaaa", "Wrong result at tryCatchConcatLoop2");
    arktest.assertEQ(tryCatchConcatLoop3("a", n), "aaaa", "Wrong result at tryCatchConcatLoop3");
    arktest.assertEQ(tryCatchConcatLoop4("a", n), "aa", "Wrong result at tryCatchConcatLoop4");
    arktest.assertEQ(tryCatchConcatLoop5("a", n), "aaaaaa", "Wrong result at tryCatchConcatLoop5");
    arktest.assertEQ(ifConcatLoop0("a", "b", n), "a1ba2b", "Wrong result at ifConcatLoop0");
    arktest.assertEQ(ifConcatLoop1("a", n), "a1a2", "Wrong result at ifConcatLoop1");
    arktest.assertEQ(ifConcatLoop2("a", "b", n), "a1bab", "Wrong result at ifConcatLoop2");
    arktest.assertEQ(ifConcatLoop3("a", "b", n), "aba2b", "Wrong result at ifConcatLoop3");
    arktest.assertEQ(ifConcatLoop4("b", n), "1b2b", "Wrong result at ifConcatLoop4");
    arktest.assertEQ(ifConcatLoop5("a", n), "a1a", "Wrong result at ifConcatLoop5");
    arktest.assertEQ(ifConcatLoop6("a", n), "aa2", "Wrong result at ifConcatLoop6");
    arktest.assertEQ(ifConcatLoop7(n), "12", "Wrong result at ifConcatLoop7");
    arktest.assertEQ(ifConcatLoop8("b", n), "b2b", "Wrong result at ifConcatLoop8");
    arktest.assertEQ(ifConcatLoop9("b", n), "1bb", "Wrong result at ifConcatLoop9");
    arktest.assertEQ(ifConcatLoop10(n), "1", "Wrong result at ifConcatLoop10");
    arktest.assertEQ(ifConcatLoop11(n), "2", "Wrong result at ifConcatLoop11");
    arktest.assertEQ(switchConcatLoop0("a", "b", n), "a0ba1b", "Wrong result at switchConcatLoop0");
    arktest.assertEQ(switchConcatLoop1("a", "b", n), "a0120ba121b", "Wrong result at switchConcatLoop1");
    arktest.assertEQ(reuseConcatLoop0("a", n), "aaa", "Wrong result at reuseConcatLoop0");
    arktest.assertEQ(reuseConcatLoop1("a", n), "0a2a", "Wrong result at reuseConcatLoop1");
    arktest.assertEQ(reuseConcatLoop2("a", n), "0a2a", "Wrong result at reuseConcatLoop2");
    arktest.assertEQ(reuseConcatLoop3("a", n), "aaaaaa", "Wrong result at reuseConcatLoop3");
    arktest.assertEQ(reuseConcatLoop4("a", n), "a1a3", "Wrong result at reuseConcatLoop4");
    arktest.assertEQ(reuseConcatLoop5("a", n), "a1a3", "Wrong result at reuseConcatLoop5");
    arktest.assertEQ(argsConcatLoop0(1.5, false, c'A', c'B'), "1.5false2AB", "Wrong result at argsConcatLoop0");
    arktest.assertEQ(bugfixConcatLoop0(), "12345678", "Wrong result at bugfixConcatLoop0");
    arktest.assertEQ(bugfixConcatLoop1(n, "ISO-8859-1"), "CC", "Wrong result at bugfixConcatLoop1");
    arktest.assertEQ(bugfixConcatLoop1(n, "UTF-16"), "힙힙", "Wrong result at bugfixConcatLoop1");
}
