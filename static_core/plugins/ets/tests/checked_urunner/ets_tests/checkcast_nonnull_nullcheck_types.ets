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

//! CHECKER      Checks Elimination keeps the classical NullCheck and throws NullPointerError
//! RUN          force_jit: true,
//!              options: "--compiler-regex='.*__noinline__classicNullCheckChain.*'",
//!              entry: "checkcast_nonnull_nullcheck_types.ETSGLOBAL::main"
//! METHOD       "checkcast_nonnull_nullcheck_types.ETSGLOBAL::__noinline__classicNullCheckChain"
//! PASS_AFTER   "ChecksElimination"
//! INST         "NullCheck"
//! INST_NOT     /NullCheck [0-9]+/
//! INST_NOT     "CheckCast"
//! INST         /LoadObject.*checkcast_nonnull_nullcheck_types\.Holder\.box/
//! INST_NEXT    "NullCheck"
//! INST_NEXT    /LoadObject.*checkcast_nonnull_nullcheck_types\.TestBox\.value/

//! CHECKER      Checks Elimination keeps the typed NullCheck and throws ClassCastError
//! RUN          force_jit: true,
//!              options: "--compiler-regex='.*__noinline__typedNullCheckChain.*'",
//!              entry: "checkcast_nonnull_nullcheck_types.ETSGLOBAL::main"
//! METHOD       "checkcast_nonnull_nullcheck_types.ETSGLOBAL::__noinline__typedNullCheckChain"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "NullCheck", 2
//! INST_COUNT   "CheckCast", 2
//! INST         /NullCheck [0-9]+/
//! PASS_AFTER   "ChecksElimination"
//! INST_COUNT   "NullCheck", 1
//! INST_COUNT   "CheckCast", 1
//! INST         /NullCheck [0-9]+/

//! CHECKER      Typed NullCheck before field access is not made implicit and throws ClassCastError
//! RUN          force_jit: true,
//!              options: "--compiler-regex='.*__noinline__typedNullCheckWithFieldAccess.*'",
//!              entry: "checkcast_nonnull_nullcheck_types.ETSGLOBAL::main"
//! EVENT_NOT    /ImplicitNullcheck,.*/
//! EVENT        /Exception,.*::__noinline__typedNullCheckWithFieldAccess,.*,CAST_CHECK/

class TestBox {
    public value: int

    constructor(value: int) {
        this.value = value
    }
}

class Holder {
    public box!: TestBox
}

function __noinline__consumeBox(_: TestBox): int {
    return 1
}

function __noinline__classicNullCheckChain(holder: Holder): int {
    try {
        let first = holder.box.value
        let second = holder.box.value
        return first + second
    } catch (e) {
        if (e instanceof NullPointerError) {
            return -1
        }
        return -2
    }
}

function __noinline__typedNullCheckChain(value: Any): int {
    let first = value as TestBox
    let second = value as TestBox
    return __noinline__consumeBox(first) + __noinline__consumeBox(second)
}

function __noinline__typedNullCheckCatch(value: Any): int {
    try {
        return __noinline__typedNullCheckChain(value)
    } catch (e) {
        if (e instanceof ClassCastError) {
            return -1
        }
        return -2
    }
}

function __noinline__typedNullCheckWithFieldAccess(value: Any): int {
    let box = value as TestBox
    return box.value
}

function __noinline__typedNullCheckWithFieldAccessCatch(value: Any): int {
    try {
        return __noinline__typedNullCheckWithFieldAccess(value)
    } catch (e) {
        if (e instanceof ClassCastError) {
            return -1
        }
        return -2
    }
}

function main(): int {
    let box = new TestBox(21)
    let holder = new Holder()
    holder.box = box

    arktest.assertEQ(__noinline__classicNullCheckChain(holder), 42)
    arktest.assertEQ(__noinline__classicNullCheckChain(new Holder()), -1)

    arktest.assertEQ(__noinline__typedNullCheckCatch(box), 2)
    arktest.assertEQ(__noinline__typedNullCheckCatch(null), -1)
    arktest.assertEQ(__noinline__typedNullCheckWithFieldAccessCatch(box), 21)
    arktest.assertEQ(__noinline__typedNullCheckWithFieldAccessCatch(null), -1)

    return 0
}
