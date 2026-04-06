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

// Source file must be last in the compile list
//! DEFINE_FRONTEND_OPTIONS DEFAULT ARGS
//! RUN_FRONTEND            inputs: "ext_checkcast_nonnull_helper.ets",
//!                         output: "ext_inlining_checkcast_nonnull_ext_checkcast_nonnull_helper.ets.abc"
//! RUN_FRONTEND            options: "--opt-level=2 --ets-strings-concat=false"

import { TestBox } from './ext_checkcast_nonnull_helper.ets'
import { castBox } from './ext_checkcast_nonnull_helper.ets'

//! CHECKER    External inlining keeps typed NullCheck from checkcast.nonnull and throws ClassCastError
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   options: "--panda-files=../ext_inlining_checkcast_nonnull_ext_checkcast_nonnull_helper.ets.abc " +
//!                     "--paoc-use-cha=true --compiler-inline-external-methods-aot=true " +
//!                     "--compiler-regex='.*__noinline__typedExternalChain.*'",
//!            entry: "ext_inlining_checkcast_nonnull.ETSGLOBAL::main"
//! EVENT      /Inline,.*::__noinline__typedExternalChain,.*::castBox,.*,STATIC,SUCCESS/
//! METHOD     "ext_inlining_checkcast_nonnull.ETSGLOBAL::__noinline__typedExternalChain"
//! PASS_BEFORE "Inline"
//! INST       /CallStatic.*ext_checkcast_nonnull_helper\.ETSGLOBAL::castBox.*\[file=\d+\]/
//! PASS_AFTER "ChecksElimination"
//! INST_COUNT "NullCheck", 1
//! INST_COUNT "CheckCast", 1
//! INST       /NullCheck [0-9]+/
//! RUN        options: "--panda-files=../ext_inlining_checkcast_nonnull_ext_checkcast_nonnull_helper.ets.abc",
//!            entry: "ext_inlining_checkcast_nonnull.ETSGLOBAL::main"

function __noinline__consumeBox(_: TestBox): int {
    return 1
}

function __noinline__typedExternalChain(value: Any): int {
    let first = castBox(value)
    let second = castBox(value)
    return __noinline__consumeBox(first) + __noinline__consumeBox(second)
}

function __noinline__typedExternalCatch(value: Any): int {
    try {
        return __noinline__typedExternalChain(value)
    } catch (e) {
        if (e instanceof ClassCastError) {
            return -1
        }
        return -2
    }
}

function main(): int {
    let box = new TestBox(7)
    arktest.assertEQ(__noinline__typedExternalCatch(box), 2)
    arktest.assertEQ(__noinline__typedExternalCatch(null), -1)
    return 0
}
