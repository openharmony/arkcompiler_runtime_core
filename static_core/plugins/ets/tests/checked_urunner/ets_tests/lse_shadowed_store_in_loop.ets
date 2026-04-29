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

//! CHECKER      Check LSE eliminates shadowed store in loop without crash
//! RUN          force_jit: true,
//!              options: "--compiler-regex='.*testLoopStore.*' " +
//!                       "--compiler-inlining-blacklist='lse_shadowed_store_in_loop.ETSGLOBAL::leakRef," +
//!                                                      "lse_shadowed_store_in_loop.Filler::fill'",
//!              entry: "lse_shadowed_store_in_loop.ETSGLOBAL::main"
//! METHOD       "lse_shadowed_store_in_loop.ETSGLOBAL::testLoopStore"
//! PASS_BEFORE  "LSE"
//! INST_NOT     /CallStatic\.Inlined.*leakRef/
//! INST         /CallStatic.*leakRef/
//! INST_COUNT   /StoreObject.*Container\.x/, 4
//! PASS_AFTER   "LSE"
//! INST_COUNT   /StoreObject.*Container\.x/, 3

//! CHECKER      Check LSE eliminates shadowed store in loop without crash (AOT)
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex='.*testLoopStore.*' " +
//!                       "--compiler-inlining-blacklist='lse_shadowed_store_in_loop.ETSGLOBAL::leakRef," +
//!                                                      "lse_shadowed_store_in_loop.Filler::fill'"
//! METHOD       "lse_shadowed_store_in_loop.ETSGLOBAL::testLoopStore"
//! PASS_BEFORE  "LSE"
//! INST_NOT     /CallStatic\.Inlined.*leakRef/
//! INST         /CallStatic.*leakRef/
//! INST_COUNT   /StoreObject.*Container\.x/, 4
//! PASS_AFTER   "LSE"
//! INST_COUNT   /StoreObject.*Container\.x/, 3
//! RUN          entry: "lse_shadowed_store_in_loop.ETSGLOBAL::main"

//! CHECKER      Check LSE eliminates shadowed store in loop without crash (AOT-PGO)
//! SKIP_IF      @architecture == "arm32"
//! RUN_PGO_PROF entry: "lse_shadowed_store_in_loop.ETSGLOBAL::main"
//! RUN_PGO_PAOC options: "--compiler-regex='.*testLoopStore.*' " +
//!                       "--compiler-inlining-blacklist='lse_shadowed_store_in_loop.ETSGLOBAL::leakRef," +
//!                                                      "lse_shadowed_store_in_loop.Filler::fill'"
//! METHOD       "lse_shadowed_store_in_loop.ETSGLOBAL::testLoopStore"
//! PASS_BEFORE  "LSE"
//! INST_NOT     /CallStatic\.Inlined.*leakRef/
//! INST         /CallStatic.*leakRef/
//! INST_COUNT   /StoreObject.*Container\.x/, 4
//! PASS_AFTER   "LSE"
//! INST_COUNT   /StoreObject.*Container\.x/, 3
//! RUN          entry: "lse_shadowed_store_in_loop.ETSGLOBAL::main"

class A {
    public value: int

    constructor(value: int) {
        this.value = value
    }
}

class Container {
    public x: A | undefined
}

function leakRef(c: Container) {}

class Filler {
    x: A

    constructor(x: A) {
        this.x = x
    }

    fill(c: Container) {
        c.x = this.x
    }
}

function testLoopStore(iters: int, filler: Filler): Container {
    let c = new Container()
    leakRef(c)  // Make sure c is not removed by PEA
    for (let i = 0; i < iters; i++) {
        const newC = new Container()
        filler.fill(newC)
        c.x = new A(iters)  // LSE removes this shadowed store; don't crash!
        c.x = newC.x        // LSE keeps this shadowing store
        c.x!.value++
    }
    return c
}

function main(): void {
    arktest.assertEQ(testLoopStore(10, new Filler(new A(2))).x!.value, 12)
}
