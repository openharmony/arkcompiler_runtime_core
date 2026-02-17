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
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

//! CHECKER       AOT IR Builder, check StringBuilder append calls merging
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*Append[0-9]*' --compiler-inlining=true"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f0Int3Append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendByte"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f1Int2Append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendByte"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f2Int1Append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendByte"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f3Int0Append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendByte"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f0ToString5Append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f1ToString4Append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f2ToString3Append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f3ToString2Append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f4ToString1Append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f5ToString0Append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "

//! CHECKER       JIT IR Builder, check StringBuilder append calls merging  (--gc-type=stw)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true,
//!               options: "--compiler-regex='.*Append[0-9]*' --compiler-inlining=true",
//!               entry: "ets_string_builder_append_merge_part3.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f0Int3Append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendByte"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f1Int2Append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendByte"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f2Int1Append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendByte"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f3Int0Append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendByte"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f0ToString5Append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f1ToString4Append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f2ToString3Append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f3ToString2Append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f4ToString1Append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//!
//! METHOD        "ets_string_builder_append_merge_part3.ETSGLOBAL::f5ToString0Append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "

//! CHECKER       JIT IR Builder, check StringBuilder append calls merging (--gc-type=g1-gc)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true,
//!               options: "--gc-type=g1-gc --no-async-jit=false --compiler-regex='.*Append[0-9]*' \
//!                         --compiler-inlining=true",
//!               entry: "ets_string_builder_append_merge_part3.ETSGLOBAL::main"

function f0Int3Append4(): string {
    let sb = new StringBuilder();
    if (sb != null) {
        sb.toString();
    }

    sb.append(0);
    sb.append('1');
    sb.append('2');
    sb.append('3');

    return sb.toString();           // applied, int+3
}

function f1Int2Append4(): string {
    let sb = new StringBuilder();
    if (sb != null) {
        sb.toString();
    }

    sb.append('0');
    sb.append(1);
    sb.append('2');
    sb.append('3');

    return sb.toString();           // applied, 1+int+2
}

function f2Int1Append4(): string {
    let sb = new StringBuilder();
    if (sb != null) {
        sb.toString();
    }

    sb.append('0');
    sb.append('1');
    sb.append(2);
    sb.append('3');

    return sb.toString();           // applied, 2+int+1
}

function f3Int0Append4(): string {
    let sb = new StringBuilder();
    if (sb != null) {
        sb.toString();
    }

    sb.append('0');
    sb.append('1');
    sb.append('2');
    sb.append(3);

    return sb.toString();           // applied, 3+int
}

function f0ToString5Append6(): string {
    let sb = new StringBuilder();
    if (sb != null) {
        sb.toString();
    }

    sb.append(sb.toString());
    sb.append('1');
    sb.append('2');
    sb.append('3');
    sb.append('4');
    sb.append('5');

    return sb.toString();           // applied, 4+2
}

function f1ToString4Append6(): string {
    let sb = new StringBuilder();
    if (sb != null) {
        sb.toString();
    }

    sb.append('0');
    sb.append(sb.toString());
    sb.append('2');
    sb.append('3');
    sb.append('4');
    sb.append('5');

    return sb.toString();           // applied, 1+4+1
}

function f2ToString3Append6(): string {
    let sb = new StringBuilder();
    if (sb != null) {
        sb.toString();
    }

    sb.append('0');
    sb.append('1');
    sb.append(sb.toString());
    sb.append('3');
    sb.append('4');
    sb.append('5');

    return sb.toString();           // applied, 2+4
}

function f3ToString2Append6(): string {
    let sb = new StringBuilder();
    if (sb != null) {
        sb.toString();
    }

    sb.append('0');
    sb.append('1');
    sb.append('2');
    sb.append(sb.toString());
    sb.append('4');
    sb.append('5');

    return sb.toString();           // applied, 3+3
}

function f4ToString1Append6(): string {
    let sb = new StringBuilder();
    if (sb != null) {
        sb.toString();
    }

    sb.append('0');
    sb.append('1');
    sb.append('2');
    sb.append('3');
    sb.append(sb.toString());
    sb.append('5');

    return sb.toString();           // applied, 4+2
}

function f5ToString0Append6(): string {
    let sb = new StringBuilder();
    if (sb != null) {
        sb.toString();
    }

    sb.append('0');
    sb.append('1');
    sb.append('2');
    sb.append('3');
    sb.append('4');
    sb.append(sb.toString());

    return sb.toString();           // applied, 4+1+1
}

function main() : int {
    arktest.assertEQ(f0Int3Append4(), '0123', 'Wrong result at f0Int3Append4');
    arktest.assertEQ(f1Int2Append4(), '0123', 'Wrong result at f1Int2Append4');
    arktest.assertEQ(f2Int1Append4(), '0123', 'Wrong result at f2Int1Append4');
    arktest.assertEQ(f3Int0Append4(), '0123', 'Wrong result at f3Int0Append4');

    arktest.assertEQ(f0ToString5Append6(), '12345', 'Wrong result at f0ToString5Append6');
    arktest.assertEQ(f1ToString4Append6(), '002345', 'Wrong result at f1ToString4Append6');
    arktest.assertEQ(f2ToString3Append6(), '0101345', 'Wrong result at f2ToString3Append6');
    arktest.assertEQ(f3ToString2Append6(), '01201245', 'Wrong result at f3ToString2Append6');
    arktest.assertEQ(f4ToString1Append6(), '012301235', 'Wrong result at f4ToString1Append6');
    arktest.assertEQ(f5ToString0Append6(), '0123401234', 'Wrong result at f5ToString0Append6');

    return 0
}
