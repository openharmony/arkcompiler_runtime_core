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

//! CHECKER      Check LSE keeps store observed by MAY_ALIAS load
//! RUN          force_jit: true,
//!              options: "--compiler-regex='.*testMayAliasObservedStore.*' " +
//!                "--compiler-inlining-blacklist='lse_shadowed_store_may_alias_load.ETSGLOBAL.Holder::nullcheck'",
//!              entry: "lse_shadowed_store_may_alias_load.ETSGLOBAL::mainMayAliasObservedStore"
//! METHOD       "lse_shadowed_store_may_alias_load.ETSGLOBAL::testMayAliasObservedStore"
//! PASS_BEFORE  "LSE"
//! INST_COUNT   /StoreObject.*Holder\.value/, 2
//! PASS_AFTER   "LSE"
//! INST_COUNT   /StoreObject.*Holder\.value/, 2

//! CHECKER      Check LSE keeps store observed by MAY_ALIAS load (AOT)
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex='.*testMayAliasObservedStore.*' " +
//!                "--compiler-inlining-blacklist='lse_shadowed_store_may_alias_load.ETSGLOBAL.Holder::nullcheck'"
//! METHOD       "lse_shadowed_store_may_alias_load.ETSGLOBAL::testMayAliasObservedStore"
//! PASS_BEFORE  "LSE"
//! INST_COUNT   /StoreObject.*Holder\.value/, 2
//! PASS_AFTER   "LSE"
//! INST_COUNT   /StoreObject.*Holder\.value/, 2
//! RUN          entry: "lse_shadowed_store_may_alias_load.ETSGLOBAL::mainMayAliasObservedStore"

class Holder {
    public value: int = 0

    public nullcheck() {}
}

function testMayAliasObservedStore(lhs: Holder, rhs: Holder): int {
    lhs.nullcheck()
    rhs.nullcheck()
    lhs.value = 1
    const witness = rhs.value
    lhs.value = 2
    return witness
}

function mainMayAliasObservedStore(): void {
    const holder = new Holder()
    const other = new Holder()
    arktest.assertEQ(testMayAliasObservedStore(holder, holder), 1)
    arktest.assertEQ(testMayAliasObservedStore(holder, other), 0)
}

//! CHECKER      Check LSE keeps read store across merge for MAY_ALIAS load
//! RUN          force_jit: true,
//!              options: "--compiler-regex='.*testMayAliasObservedStoreAcrossBlocks.*'",
//!              entry: "lse_shadowed_store_may_alias_load.ETSGLOBAL::mainMayAliasObservedStoreAcrossBlocks"
//! METHOD       "lse_shadowed_store_may_alias_load.ETSGLOBAL::testMayAliasObservedStoreAcrossBlocks"
//! PASS_BEFORE  "LSE"
//! INST_COUNT   /StoreObject.*Holder\.value/, 3
//! PASS_AFTER   "LSE"
//! INST_COUNT   /StoreObject.*Holder\.value/, 3

//! CHECKER      Check LSE keeps read store across merge for MAY_ALIAS load (AOT)
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex='.*testMayAliasObservedStoreAcrossBlocks.*'"
//! METHOD       "lse_shadowed_store_may_alias_load.ETSGLOBAL::testMayAliasObservedStoreAcrossBlocks"
//! PASS_BEFORE  "LSE"
//! INST_COUNT   /StoreObject.*Holder\.value/, 3
//! PASS_AFTER   "LSE"
//! INST_COUNT   /StoreObject.*Holder\.value/, 3
//! RUN          entry: "lse_shadowed_store_may_alias_load.ETSGLOBAL::mainMayAliasObservedStoreAcrossBlocks"

function testMayAliasObservedStoreAcrossBlocks(a: Holder, b: Holder, flag: boolean) {
    let res = 0;
    a.value = 10;
    if (flag) {
        res = b.value;
        a.value = 20;
    }
    a.value = 30;
    return res;
}

function mainMayAliasObservedStoreAcrossBlocks(): void {
    const holder = new Holder()
    const other = new Holder()
    arktest.assertEQ(testMayAliasObservedStoreAcrossBlocks(holder, holder, true), 10)
    arktest.assertEQ(testMayAliasObservedStoreAcrossBlocks(holder, holder, false), 0)
    arktest.assertEQ(testMayAliasObservedStoreAcrossBlocks(holder, other, true), 0)
    arktest.assertEQ(testMayAliasObservedStoreAcrossBlocks(holder, other, false), 0)
}
