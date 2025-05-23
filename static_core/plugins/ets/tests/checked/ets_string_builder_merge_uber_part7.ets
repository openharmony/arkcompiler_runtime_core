/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

//! CHECKER       JIT IR Builder, check String Builders merging
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--panda-files=../../ets_string_builder_merge_uber_export.checked/test.abc --compiler-regex='.*chain_concat[0-9]+.*' --compiler-inlining=false", entry: "ets_string_builder_merge_uber_part7.ETSGLOBAL::main"

//! CHECKER       JIT IR Builder, check String Builders merging (w/o optimizations)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--panda-files=../../ets_string_builder_merge_uber_export.checked/test.abc --compiler-inlining=false --compiler-simplify-string-builder=false", entry: "ets_string_builder_merge_uber_part7.ETSGLOBAL::main"

import { uber_chain_concat4 } from "./ets_string_builder_merge_uber_export.ets"

const uber_golden_TTFF = "_aaaa_bdbdbd_aaaa_cacaca_ddddd_aaaa_cacaca_dbdbdb_aaaaa_dd_dbdbdb_aaaa_cacaca_dbdbdb_aaaaa_aa_";
const uber_golden_TTFT = "_aaaa_badbadbad_aaaa_cbacbacba_ddddd_aaaa_cbacbacba_dcbdcbdcb_aaaaa_dd_dcbdcbdcb_aaaa_cbacbacba_dcbdcbdcb_aaaaa_aa_";

function main() {
    assertEQ(uber_chain_concat4(false, "a", 4, "b", 3, "c", 5, "d", 2, true, true, false, false)  , uber_golden_TTFF, "Wrong result at uber_golden_TTFF")
    assertEQ(uber_chain_concat4(false, "a", 4, "b", 3, "c", 5, "d", 2, true, true, false, true)   , uber_golden_TTFT, "Wrong result at uber_golden_TTFT")
}
